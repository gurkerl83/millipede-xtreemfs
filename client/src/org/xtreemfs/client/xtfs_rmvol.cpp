// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "org/xtreemfs/client.h"
#include "main.h"
using namespace org::xtreemfs::client;

#include "yield/platform.h"


namespace org
{
  namespace xtreemfs
  {
    namespace client
    {
      class xtfs_rmvol : public Main
      {
      public:
        xtfs_rmvol()
          : Main( "xtfs_rmvol", "remove a volume from a specified MRC", "[oncrpc[s]://]<mrc host>[:port]/<volume name>" )
        { }

      private:
        std::auto_ptr<YIELD::URI> mrc_uri;
        std::string volume_name;


        // xtfs_bin
        int _main( int, char** )
        {
          YIELD::auto_Object<MRCProxy> mrc_proxy = createMRCProxy( *mrc_uri.get() );
          mrc_proxy.get()->rmvol( volume_name );
          return 0;
        }

        void parseFiles( int files_count, char** files )
        {
          if ( files_count >= 1 )
          {
            mrc_uri = parseURI( files[0] );
            if ( strlen( mrc_uri.get()->get_resource() ) > 1 )
              volume_name = mrc_uri.get()->get_resource() + 1;
          }
          else
            throw YIELD::Exception( "must specify the MRC and volume name as a URI" );
        }
      };
    };
  };
};


int main( int argc, char** argv )
{
  return xtfs_rmvol().main( argc, argv );
}
