// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef ORG_XTREEMFS_CLIENT_DIR_PROXY_H
#define ORG_XTREEMFS_CLIENT_DIR_PROXY_H

#include "org/xtreemfs/client/proxy.h"
#include "org/xtreemfs/interfaces/dir_interface.h"
#include "org/xtreemfs/client/proxy.h"

#include <map>
#include <string>


namespace org
{
  namespace xtreemfs
  {
    namespace client
    {
      class PolicyContainer;


      class DIRProxy : public Proxy
      {
      public:
        ORG_XTREEMFS_CLIENT_PROXY_CONSTRUCTORS( DIRProxy, dir_interface );
        virtual ~DIRProxy();

        YIELD::URI getURIFromUUID( const std::string& uuid );
        YIELD::URI getVolumeURIFromVolumeName( const std::string& volume_name );

      private:
        org::xtreemfs::interfaces::DIRInterface dir_interface;
        PolicyContainer* policies;

        class CachedAddressMappingURI : public YIELD::URI
        {
        public:
          CachedAddressMappingURI( const std::string& uri, uint32_t ttl_s )
            : YIELD::URI( uri ), ttl_s( ttl_s )
          {
            creation_epoch_time_s = YIELD::Time::getCurrentUnixTimeS();
          }

          uint32_t get_ttl_s() const { return ttl_s; }
          double get_creation_epoch_time_s() const { return creation_epoch_time_s; }

        private:
          uint32_t ttl_s;
          double creation_epoch_time_s;
        };

        std::map<std::string, CachedAddressMappingURI*> uuid_to_uri_cache;
        YIELD::Mutex uuid_to_uri_cache_lock;
      };
    };
  };
};

#endif
