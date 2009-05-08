// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef ORG_XTREEMFS_CLIENT_VOLUME_H
#define ORG_XTREEMFS_CLIENT_VOLUME_H

#include "org/xtreemfs/client/dir_proxy.h"
#include "org/xtreemfs/client/mrc_proxy.h"
#include "org/xtreemfs/client/path.h"


namespace org
{
  namespace xtreemfs
  {
    namespace client
    {
      class OSDProxy;


      class Volume : public YIELD::Volume
      {
      public:
        const static uint32_t VOLUME_FLAG_CACHE_FILES = 1;
        const static uint32_t VOLUME_FLAG_CACHE_METADATA = 2;
        
        Volume( const YIELD::SocketAddress& dir_sockaddr, const std::string& name, YIELD::auto_Object<YIELD::SocketFactory> socket_factory = NULL, uint32_t flags = 0, YIELD::auto_Object<YIELD::Log> log = NULL );

        // Callbacks for File
        uint32_t get_flags() const { return flags; }
        YIELD::auto_Object<OSDProxy> get_osd_proxy( const std::string& osd_uuid );

        // YIELD::Volume
        YIELD_PLATFORM_VOLUME_PROTOTYPES;
        YIELD::auto_Object<YIELD::Stat> getattr( const Path& path );
        bool listdir( const YIELD::Path& path, listdirCallback& callback ) { return listdir( path, Path(), callback ); }
        bool listdir( const YIELD::Path& path, const YIELD::Path& match_file_name_prefix, listdirCallback& callback );
        
      private:
        ~Volume();

        YIELD::auto_Object<YIELD::StageGroup> stage_group;
        std::string name;
        YIELD::auto_Object<DIRProxy> dir_proxy;
        YIELD::auto_Object<MRCProxy> mrc_proxy;
        uint32_t flags;
        YIELD::auto_Object<YIELD::Log> log;

        YIELD::STLHashMap<OSDProxy*> osd_proxy_cache;
        YIELD::Mutex osd_proxy_cache_lock;

        void osd_unlink( const org::xtreemfs::interfaces::FileCredentialsSet& );
        void set_errno( const char* operation_name, ProxyExceptionResponse& proxy_exception_response );
        void set_errno( const char* operation_name, std::exception& exc );
      };
    };
  };
};

#endif