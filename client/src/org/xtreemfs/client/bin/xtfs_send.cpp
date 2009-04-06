// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "org/xtreemfs/client.h"
#include "options.h"
using namespace org::xtreemfs::client;

#include "yield.h"

#include <iostream>
using std::cout;
using std::endl;


namespace org
{
  namespace xtreemfs
  {
    namespace client
    {
      class argvInputStream : public YIELD::StructuredInputStream
      {
      public:
        argvInputStream( int argc, char** argv ) : argc( argc ), argv( argv )
        {
          next_arg_i = 0;
        }

        bool readBool( const Declaration& decl )
        {
          const char* value = readValue( decl );
          return value && ( strcmp( value, "true" ) == 0 || strcmp( value, "t" ) == 0 );
        }

        double readDouble( const Declaration& decl )
        {
          const char* value = readValue( decl );
          if ( value )
            return atof( value );
          else
            return 0;
        }

        int64_t readInt64( const Declaration& decl )
        {
          const char* value = readValue( decl );
          if ( value )
            return atol( value );
          else
            return 0;
        }

        YIELD::Serializable* readSerializable( const Declaration& decl, YIELD::Serializable* s = NULL )
        {
          if ( s )
          {
            switch ( s->getGeneralType() )
            {
              case YIELD::RTTI::STRING: readString( decl, static_cast<YIELD::SerializableString&>( *s ) ); break;
              case YIELD::RTTI::STRUCT: s->deserialize( *this ); break;
            }
          }

          return s;
        }

        void readString( const Declaration& decl, std::string& str )
        {
          const char* value = readValue( decl );
          if ( value )
            str = value;
        }

        uint64_t readUint64( const Declaration& decl )
        {
          const char* value = readValue( decl );
          if ( value )
            return atol( value );
          else
            return 0;
        }

      private:
        int argc;
        char** argv;
        int next_arg_i;

        const char* readValue( const Declaration& decl )
        {
          if ( next_arg_i < argc )
            return argv[next_arg_i++];
          else
            return NULL;
        }
      };


      class xtfs_sendOptions : public Options
      {
      public:
        xtfs_sendOptions( int argc, char** argv )
          : Options( "xtfs_send", "send RPCs to an XtreemFS server", "[oncrpc[s]://]<host>[:port]/<rpc operation name> [rpc operation parameters]" )
        {
          org::xtreemfs::interfaces::DIRInterface().registerSerializableFactories( serializable_factories );
          org::xtreemfs::interfaces::MRCInterface().registerSerializableFactories( serializable_factories );
          org::xtreemfs::interfaces::OSDInterface().registerSerializableFactories( serializable_factories );

          request = NULL;
          proxy = NULL;

          parseOptions( argc, argv );
        }

        ~xtfs_sendOptions()
        {
          YIELD::SharedObject::decRef( request );
          delete proxy;
        }

        Proxy& get_proxy() const { return *proxy; }
        YIELD::Request& get_request() const { return *request; }

      private:
        YIELD::SerializableFactories serializable_factories;
        YIELD::Request* request;
        Proxy* proxy;

        // OptionParser
        void parseFiles( int files_count, char** files )
        {
          if ( files_count >= 1 )
          {
            std::auto_ptr<YIELD::URI> rpc_uri( parseURI( files[0] ) );

            if ( strlen( rpc_uri.get()->get_resource() ) > 1 )
            {
              std::string request_type_name( rpc_uri.get()->get_resource() + 1 );
              request = static_cast<YIELD::Request*>( serializable_factories.createSerializable( "org::xtreemfs::interfaces::MRCInterface::" + request_type_name + "SyncRequest" ) );
              if ( request != NULL )
                proxy = createProxy<MRCProxy>( *rpc_uri.get() );
              else
              {
                request = static_cast<YIELD::Request*>( serializable_factories.createSerializable( "org::xtreemfs::interfaces::DIRInterface::" + request_type_name + "SyncRequest" ) );
                if ( request != NULL )
                  proxy = createProxy<DIRProxy>( *rpc_uri.get() );
                else
                {
                  request = static_cast<YIELD::Request*>( serializable_factories.createSerializable( "org::xtreemfs::interfaces::OSDInterface::" + request_type_name + "SyncRequest" ) );
                  if ( request != NULL )
                    proxy = createProxy<OSDProxy>( *rpc_uri.get() );
                  else
                    throw YIELD::Exception( "unknown operation" );
                }
              }

              if ( files_count > 1 )
              {
                argvInputStream argv_input_stream( files_count - 1, files+1 );
                request->deserialize( argv_input_stream );
              }
            }
            else
              throw YIELD::Exception( "RPC URI must include an operation name" );
          }
          else
            throw YIELD::Exception( "must specify RPC URI" );
        }
      };
    };
  };
};

int main( int argc, char** argv )
{
  try
  {
    xtfs_sendOptions options( argc, argv );

    if ( options.get_help() )
      options.printUsage();
    else
    {
      Proxy& proxy = options.get_proxy();
      YIELD::Request& req = options.get_request();
      YIELD::SharedObject::incRef( req );
      proxy.send( req );

      YIELD::Event& resp = req.waitForDefaultResponse( options.get_timeout_ms() );
      std::cout << resp.getTypeName() << "( ";
      YIELD::PrettyPrintOutputStream output_stream( std::cout );
      resp.serialize( output_stream );
      std::cout << " )" << std::endl;
      YIELD::SharedObject::decRef( resp );
    }

    return 0;
  }
  catch ( std::exception& exc )
  {
    std::cerr << "Error on request: " << exc.what() << std::endl;

    return 1;
  }
}
