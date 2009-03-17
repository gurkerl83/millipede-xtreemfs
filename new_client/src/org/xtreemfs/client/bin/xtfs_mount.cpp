#include "org/xtreemfs/client.h"
using namespace org::xtreemfs::client;

#include "yield.h"

#include "yieldfs/fuse.h"
#include "yieldfs/time_cached_volume.h"
#include "yieldfs/local_volume.h"

#include <string>
#include <vector>
#include <exception>
#include <iostream>

#include "SimpleOpt.h"


enum { OPT_FOREGROUND, OPT_DEBUG, OPT_DIR, OPT_OLD, OPT_VOLUME_URI, OPT_VOLUME_UUID };

CSimpleOpt::SOption options[] = {
  { OPT_DEBUG, "-d", SO_NONE },
  { OPT_DEBUG, "--debug", SO_NONE },
  { OPT_DIR, "--dir", SO_REQ_SEP },
  { OPT_DIR, "--dirservice", SO_REQ_SEP },
  { OPT_FOREGROUND, "-f", SO_NONE },
  { OPT_OLD, "-o", SO_REQ_SEP },
  { OPT_VOLUME_URI, "--volume-url", SO_REQ_SEP },
  { OPT_VOLUME_URI, "--volume-uri", SO_REQ_SEP },
  { OPT_VOLUME_UUID, "--volume", SO_REQ_SEP },
  { OPT_VOLUME_UUID, "--volume-uuid", SO_REQ_SEP },
  { OPT_VOLUME_UUID, "--volume_name", SO_REQ_SEP },
  SO_END_OF_OPTIONS
};


int main( int argc, char** argv )
{
  // Options to fill
  bool foreground = false, debug = false;
  std::string dir, mount_point, volume_uri_str, volume_name;
  YIELD::URI *dir_uri = NULL, *mrc_uri = NULL;

  try
  {
    CSimpleOpt args( argc, argv, options );

    // - options
    while ( args.Next() )
    {
      if ( args.LastError() == SO_SUCCESS )
      {
        switch ( args.OptionId() )
        {
          case OPT_FOREGROUND: foreground = true; break;
          case OPT_DEBUG: debug = true; break;
          case OPT_DIR: dir = args.OptionArg(); break;

          case OPT_VOLUME_UUID: volume_name = args.OptionArg(); break;
          case OPT_VOLUME_URI: volume_uri_str = args.OptionArg(); break;

          case OPT_OLD:
          {
            std::string old_args_str( args.OptionArg() );
            std::vector<std::string> old_args;

            std::string::size_type last_comma = old_args_str.find_first_not_of( ",", 0 );
            std::string::size_type comma = old_args_str.find_first_of( ",", last_comma);
            while ( comma != std::string::npos || last_comma != std::string::npos )
            {
              old_args.push_back( old_args_str.substr( last_comma, comma - last_comma ) );
              last_comma = old_args_str.find_first_not_of( ",", comma );
              comma = old_args_str.find_first_of( ",", last_comma );
            }

            for ( std::vector<std::string>::iterator old_arg_i = old_args.begin(); old_arg_i != old_args.end(); old_arg_i++ )
            {
              std::string old_arg_key, old_arg_value;
              std::string::size_type equals = ( *old_arg_i ).find_first_of( "=" );
              if ( equals == std::string::npos || equals == ( *old_arg_i ).size() - 1 )
                old_arg_key = *old_arg_i;
              else
              {
                old_arg_key = ( *old_arg_i ).substr( 0, equals );
                old_arg_value = ( *old_arg_i ).substr( equals + 1 );
              }

              if ( old_arg_key == "volume_url" )
                volume_uri_str = old_arg_value;
              else if ( old_arg_key == "dirservice" )
                dir = old_arg_value;
              else if ( old_arg_key == "debug" )
                debug = true;
            }
          }
        }
      }
    }

    // [dirservice] [volume_url] mount point after - options
    if ( args.FileCount() == 0 )
      throw YIELD::Exception( "must specify dirservice, volume_url, and mount point" );
    switch ( args.FileCount() )
    {
      case 1: mount_point = args.Files()[0]; break;
      case 2: volume_name = args.Files()[0]; mount_point = args.Files()[1]; break;
      case 3: dir = args.Files()[0]; volume_name = args.Files()[1]; mount_point = args.Files()[2]; break;
    }

    if ( !dir.empty() )
    {
      dir_uri = YIELD::URI::parseURI( dir );
      if ( dir_uri == NULL ) // Assume dir is a host[:port]
      {
        std::string complete_dir( dir );
        if ( complete_dir.find( "://" ) != std::string::npos )
          complete_dir = "oncrpc://" + complete_dir;
        dir_uri = YIELD::URI::parseURI( complete_dir );        
        if ( dir_uri == NULL )
          throw YIELD::Exception( "invalid directory service URI" );
      }
    }
    else
      throw YIELD::Exception( "must specify directory service URI" );

    if ( !volume_uri_str.empty() )
    {
        mrc_uri = YIELD::URI::parseURI( volume_uri_str );
        if ( mrc_uri != NULL )
        {
          if ( strlen( mrc_uri->getResource() ) >= 2 )
            volume_name = mrc_uri->getResource();
          else
            throw YIELD::Exception( "must specify volume name in volume URI" );
        }
        else
          volume_name = volume_uri_str;
    }
    else if ( volume_name.empty() )
      throw YIELD::Exception( "must specify volume" );

    if ( mount_point.empty() )
      throw YIELD::Exception( "must specify mount point" );

    if ( debug )
      YIELD::SocketConnection::setTraceSocketIO( true );

    YIELD::SEDAStageGroup& main_stage_group = YIELD::SEDAStageGroup::createStageGroup();

    uint32_t dir_proxy_flags = debug ? Proxy::PROXY_FLAG_PRINT_OPERATIONS : 0;
    DIRProxy dir_proxy( *dir_uri, 3, dir_proxy_flags ); main_stage_group.createStage( dir_proxy );

    if ( mrc_uri == NULL )
    {
      org::xtreemfs::interfaces::ServiceRegistrySet service_registries;
      dir_proxy.service_get_by_name( volume_name, service_registries );
      if ( !service_registries.empty() )
      {
        for ( org::xtreemfs::interfaces::ServiceRegistrySet::const_iterator service_registry_i = service_registries.begin(); service_registry_i != service_registries.end(); service_registry_i++ )
        {
          const org::xtreemfs::interfaces::ServiceRegistryDataMap& data = ( *service_registry_i ).get_data();
          for ( org::xtreemfs::interfaces::ServiceRegistryDataMap::const_iterator data_i = data.begin(); data_i != data.end(); data_i++ )
          {
            if ( data_i->first == "mrc" )
            {
              mrc_uri = new YIELD::URI( dir_proxy.get_uri_from_uuid( data_i->second ) );
              break;
            }
          }

          if ( mrc_uri != NULL )
            break;
        }
      }
      else
        throw YIELD::Exception( "unknown volume" );
    }

    uint32_t mrc_proxy_flags = debug ? Proxy::PROXY_FLAG_PRINT_OPERATIONS : 0;
    MRCProxy mrc_proxy( *mrc_uri, 3, mrc_proxy_flags ); main_stage_group.createStage( mrc_proxy );

    uint32_t osd_proxy_flags = debug ? Proxy::PROXY_FLAG_PRINT_OPERATIONS : 0;
    OSDProxyFactory osd_proxy_factory( dir_proxy, main_stage_group, 33, osd_proxy_flags );

    Volume xtreemfs_volume( volume_name, dir_proxy, mrc_proxy, osd_proxy_factory );
 //   yieldfs::LocalVolume xtreemfs_volume( YIELD::Path(), "test" );
 //   yieldfs::TimeCachedVolume time_cached_volume( xtreemfs_volume );
    int ret = yieldfs::FUSE( xtreemfs_volume ).main( argv[0], mount_point.c_str(), foreground, debug );

    YIELD::SEDAStageGroup::destroyStageGroup( main_stage_group ); // Must destroy the stage group before the event handlers go out of scope so the stages aren't holding dead pointers

    delete dir_uri;
    delete mrc_uri;

    return ret;
  }
  catch ( std::exception& exc )
  {
    std::cerr << "Error mounting volume: " << exc.what() << std::endl;

    delete dir_uri;
    delete mrc_uri;

    return 1;
  }
}
