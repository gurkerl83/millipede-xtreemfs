// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef _XTREEMFS_MRC_PROXY_H_
#define _XTREEMFS_MRC_PROXY_H_

#include "xtreemfs/proxy.h"

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable: 4100 )
#endif
#include "xtreemfs/interfaces/mrc_interface.h"
#ifdef _WIN32
#pragma warning( pop )
#endif


namespace xtreemfs
{
  class Path;


  class MRCProxy : public Proxy<MRCProxy, org::xtreemfs::interfaces::MRCInterface>
  {
  public:
    static yidl::auto_Object<MRCProxy> create( const YIELD::URI& absolute_uri,
                                                uint32_t flags = 0,
                                                YIELD::auto_Log log = NULL,
                                                const YIELD::Time& operation_timeout = YIELD::ONCRPCClient<org::xtreemfs::interfaces::MRCInterface>::OPERATION_TIMEOUT_DEFAULT,
                                                YIELD::auto_SSLContext ssl_context = NULL )
    {
      return YIELD::ONCRPCClient<org::xtreemfs::interfaces::MRCInterface>::create<MRCProxy>( absolute_uri, flags, log, operation_timeout, ssl_context );
    }

    // org::xtreemfs::interfaces::MRCInterface
    void chown( const Path& path, int uid, int gid );
    void getattr( const Path& path, org::xtreemfs::interfaces::Stat& stbuf );
    void readdir( const Path& path, org::xtreemfs::interfaces::DirectoryEntrySet& directory_entries );

  private:
    friend class YIELD::ONCRPCClient<org::xtreemfs::interfaces::MRCInterface>;

    MRCProxy( const YIELD::URI& absolute_uri, uint32_t flags, YIELD::auto_Log log, const YIELD::Time& operation_timeout, YIELD::Socket::auto_Address peer_sockaddr, YIELD::auto_SSLContext ssl_context )
      : Proxy<MRCProxy, org::xtreemfs::interfaces::MRCInterface>( absolute_uri, flags, log, operation_timeout, peer_sockaddr, ssl_context )
    { }

    ~MRCProxy() { }
  };

  typedef yidl::auto_Object<MRCProxy> auto_MRCProxy;
};

#endif
