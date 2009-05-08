// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "main.h"
using namespace org::xtreemfs::client;

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

        YIELD::Object* readObject( const Declaration& decl, YIELD::Object* value = NULL, YIELD::Object::GeneralType value_general_type = YIELD::Object::UNKNOWN )
        {
          if ( value )
          {
            if ( value_general_type == YIELD::Object::UNKNOWN )
              value_general_type = value->get_general_type();

            switch ( value_general_type )
            {
              case YIELD::Object::STRING: readString( decl, static_cast<YIELD::String&>( *value ) ); break;
              case YIELD::Object::STRUCT: value->deserialize( *this ); break;
              default: YIELD::DebugBreak(); break;
            }
          }

          return value;
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


      class xtfs_send : public Main
      {
      public:
        xtfs_send()
          : Main( "xtfs_send", "send RPCs to an XtreemFS server", "[oncrpc[s]://]<host>[:port]/<rpc operation name> [rpc operation parameters]" )
        {
          object_factories = new YIELD::ObjectFactories;
          org::xtreemfs::interfaces::DIRInterface().registerObjectFactories( *object_factories );
          org::xtreemfs::interfaces::MRCInterface().registerObjectFactories( *object_factories );
          org::xtreemfs::interfaces::OSDInterface().registerObjectFactories( *object_factories );
        }

      private:
        YIELD::auto_Object<YIELD::ObjectFactories> object_factories;
        YIELD::auto_Object<YIELD::Request> request;
        YIELD::auto_Object<YIELD::EventTarget> proxy;

        // YIELD::Main
        int _main( int, char** )
        {
          proxy->send( request->incRef() );

          YIELD::Event& resp = request->waitForDefaultResponse( static_cast<uint64_t>( -1 ) ); // get_timeout_ms() );
/*
          std::cout << resp.get_type_name() << "( ";
          YIELD::PrettyPrintOutputStream output_stream( std::cout );
          resp.serialize( output_stream );
          std::cout << " )" << std::endl;
*/
          YIELD::Object::decRef( resp );

          return 0;
        }

        void parseFiles( int files_count, char** files )
        {
          if ( files_count >= 1 )
          {
            YIELD::auto_Object<YIELD::URI> rpc_uri = parseURI( files[0] );

            if ( rpc_uri->get_resource().size() > 1 )
            {
              std::string request_type_name( rpc_uri->get_resource().c_str() + 1 );
              request = static_cast<YIELD::Request*>( object_factories->createObject( "org::xtreemfs::interfaces::MRCInterface::" + request_type_name + "SyncRequest" ).release() );
              if ( request!= NULL )
                proxy = createMRCProxy( *rpc_uri ).release();
              else
              {
                request = static_cast<YIELD::Request*>( object_factories->createObject( "org::xtreemfs::interfaces::DIRInterface::" + request_type_name + "SyncRequest" ).release() );
                if ( request!= NULL )
                  proxy = createDIRProxy( *rpc_uri ).release();
                else
                {
                  request = static_cast<YIELD::Request*>( object_factories->createObject( "org::xtreemfs::interfaces::OSDInterface::" + request_type_name + "SyncRequest" ).release() );
                  if ( request!= NULL )
                    proxy = createOSDProxy( *rpc_uri ).release();
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
  return xtfs_send().main( argc, argv );
}