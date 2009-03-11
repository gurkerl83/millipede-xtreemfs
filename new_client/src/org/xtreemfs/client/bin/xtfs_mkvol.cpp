#include "org/xtreemfs/client.h"
using namespace org::xtreemfs::client;

#include "yield/platform.h"

#include <string>
#include <vector>
#include <exception>
#include <iostream>
using namespace std;

#include "SimpleOpt.h"


enum { OPT_DEBUG, OPT_STRIPING_POLICY, OPT_OSD_SELECTION, OPT_ACCESS_POLICY, OPT_MODE, OPT_CERT, OPT_DIRSERVICE, OPT_HELP };

CSimpleOpt::SOption options[] = {
  { OPT_DEBUG, "-d", SO_NONE },
  { OPT_STRIPING_POLICY, "-p", SO_REQ_SEP },
  { OPT_STRIPING_POLICY, "--striping-policy", SO_REQ_SEP },
  { OPT_OSD_SELECTION, "-s", SO_REQ_SEP },
  { OPT_OSD_SELECTION, "--osd-selection", SO_REQ_SEP },
  { OPT_ACCESS_POLICY, "-a", SO_REQ_SEP },
  { OPT_ACCESS_POLICY, "--access-policy", SO_REQ_SEP },
  { OPT_MODE, "-m", SO_REQ_SEP },
  { OPT_MODE, "--mode", SO_REQ_SEP },
  { OPT_CERT, "-c", SO_REQ_SEP },
  { OPT_CERT, "--cert", SO_REQ_SEP },
  { OPT_DIRSERVICE, "-D", SO_REQ_SEP },
  { OPT_DIRSERVICE, "--dirservice", SO_REQ_SEP },
  { OPT_HELP, "-h", SO_REQ_SEP },
  { OPT_HELP, "--help", SO_REQ_SEP },
  SO_END_OF_OPTIONS
};

int main( int argc, char** argv )
{
  int ret = 0;

  // Options to fill
  bool debug = false;
  string volume_url; YIELD::URI* parsed_volume_url = NULL;
  uint8_t striping_policy_id = org::xtreemfs::interfaces::STRIPING_POLICY_DEFAULT; size_t striping_policy_size = 4, striping_policy_width = 1;
  int osd_selection = 1, access_policy = 2, mode = 0;
  string cert_file, dirservice;

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
          case OPT_DEBUG: debug = true; break;

          case OPT_STRIPING_POLICY:
          {
            if ( strncmp( args.OptionArg(), "RAID", 4 ) == 0 && strlen( args.OptionArg() ) > 4 )
            {
              switch ( args.OptionArg()[4] )
              {
                case '0':
                case '1':
                case '5': break; // Set striping_policy_id
              }
            }
          }
          break;

          case OPT_OSD_SELECTION:
          {
            osd_selection = atoi( args.OptionArg() );
            if ( osd_selection < 1 || osd_selection > 2 )
              osd_selection = 1;
          }
          break;

          case OPT_ACCESS_POLICY:
          {
            access_policy = atoi( args.OptionArg() );
            if ( access_policy < 1 || access_policy > 3 )
              access_policy = 2;
          }
          break;

          case OPT_MODE: mode = atoi( args.OptionArg() ); break;
          case OPT_CERT: cert_file = args.OptionArg(); break;
          case OPT_DIRSERVICE: dirservice = args.OptionArg(); break;

          case OPT_HELP: YIELD::DebugBreak(); break;
        }
      }
    }

    // volume_url after - options
    if ( args.FileCount() >= 1 )
      volume_url = args.Files()[0];
    else
      throw YIELD::Exception( "must specify volume_url" );

    parsed_volume_url = new YIELD::URI( volume_url );
    if ( strlen( parsed_volume_url->getResource() ) <= 1 )
      throw YIELD::Exception( "volume URL must include a volume name" );
  }
  catch ( exception& exc )
  {
    cerr << "Error parsing command line arguments: " << exc.what() << endl;
    ret = 1;
  }

  if ( ret == 0 )
  {
    if ( debug )
      YIELD::SocketConnection::setTraceSocketIO( true );  

    try
    {
      MRCProxy mrc_proxy( *parsed_volume_url );
      mrc_proxy.mkvol( org::xtreemfs::interfaces::Context( "user", org::xtreemfs::interfaces::StringSet() ), "", parsed_volume_url->getResource()+1, org::xtreemfs::interfaces::OSD_SELECTION_POLICY_SIMPLE, org::xtreemfs::interfaces::StripingPolicy( striping_policy_id, striping_policy_size, striping_policy_width ), org::xtreemfs::interfaces::ACCESS_CONTROL_POLICY_NULL );
    }
    catch ( ProxyException& exc )
    {
      cerr << "Error creating volume: " << exc.what() << endl;
      // cerr << "  exceptionName: " << exc.get_exceptionName() << endl;
      cerr << "  errno: " << exc.get_error_code() << endl;
      // cerr << "  stackTrace: " << exc.get_stackTrace() << endl;
    }
    catch ( exception& exc )
    {
      cerr << "Error creating volume: " << exc.what() << endl;  
    }
  }
  
  delete parsed_volume_url;
  return ret;
}