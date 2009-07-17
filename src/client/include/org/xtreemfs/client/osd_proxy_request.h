// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef _ORG_XTREEMFS_CLIENT_OSD_PROXY_REQUEST_H_
#define _ORG_XTREEMFS_CLIENT_OSD_PROXY_REQUEST_H_

#include "yield.h"


namespace org
{
  namespace xtreemfs
  {
    namespace client
    {
      class OSDProxyRequest : public YIELD::Request
      {
      public:
        size_t get_selected_file_replica() const { return selected_file_replica; }
        void set_selected_file_replica( size_t selected_file_replica ) { this->selected_file_replica = selected_file_replica; }

      protected:
        OSDProxyRequest()
          : selected_file_replica( SIZE_MAX )
        { }

      private:
        size_t selected_file_replica;
      };
    };
  };
};

#define ORG_XTREEMFS_INTERFACES_OSDINTERFACE_REQUEST_PARENT_CLASS org::xtreemfs::client::OSDProxyRequest

#endif