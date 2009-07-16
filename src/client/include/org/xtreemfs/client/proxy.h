// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef _ORG_XTREEMFS_CLIENT_PROXY_H_
#define _ORG_XTREEMFS_CLIENT_PROXY_H_

#include "org/xtreemfs/client/proxy_exception_response.h"
#include "org/xtreemfs/client/policy.h"
#include "org/xtreemfs/interfaces/types.h"

#include "yield.h"


namespace org
{
  namespace xtreemfs
  {
    namespace client
    {
      template <class ProxyType, class InterfaceType>
      class Proxy : public YIELD::ONCRPCClient<InterfaceType>
      {
      public:
        const static uint32_t PROXY_FLAG_TRACE_IO = YIELD::Peer<YIELD::ONCRPCRequest, YIELD::ONCRPCResponse>::PEER_FLAG_TRACE_IO;
        const static uint32_t PROXY_FLAG_TRACE_OPERATIONS = YIELD::Peer<YIELD::ONCRPCRequest, YIELD::ONCRPCResponse>::PEER_FLAG_TRACE_OPERATIONS;
        const static uint32_t PROXY_FLAG_TRACE_AUTH = 8;

        // YIELD::EventTarget
        virtual void send( YIELD::Event& ev );
          
      protected:
        Proxy( const YIELD::URI& absolute_uri, uint32_t flags, YIELD::auto_Log log, uint8_t operation_retries_max, const YIELD::Time& operation_timeout, YIELD::auto_Object<YIELD::SocketAddress> peer_sockaddr, YIELD::auto_Object<YIELD::SSLContext> ssl_context );
        virtual ~Proxy();

        void getCurrentUserCredentials( org::xtreemfs::interfaces::UserCredentials& out_user_credentials );
#ifndef _WIN32
        void getpasswdFromUserCredentials( const std::string& user_id, const std::string& group_id, int& out_uid, int& out_gid );
        bool getUserCredentialsFrompasswd( int uid, int gid, org::xtreemfs::interfaces::UserCredentials& out_user_credentials );        
#endif

      private:
        YIELD::auto_Log log;

#ifndef _WIN32
        get_passwd_from_user_credentials_t get_passwd_from_user_credentials;
        YIELD::STLHashMap<YIELD::STLHashMap<std::pair<int, int>*>*> user_credentials_to_passwd_cache;

        get_user_credentials_from_passwd_t get_user_credentials_from_passwd;
        YIELD::STLHashMap<YIELD::STLHashMap<org::xtreemfs::interfaces::UserCredentials*>*> passwd_to_user_credentials_cache;
#endif

        std::vector<YIELD::SharedLibrary*> policy_shared_libraries;

        template <typename PolicyFunctionType> 
        bool getPolicyFunction( const YIELD::Path& policy_shared_library_path, YIELD::auto_Object<YIELD::SharedLibrary> policy_shared_library, const char* policy_function_name, PolicyFunctionType& out_policy_function )
        {
          PolicyFunctionType policy_function = policy_shared_library->getFunction<PolicyFunctionType>( policy_function_name );
          if ( policy_function != NULL )
          {
            log->getStream( YIELD::Log::LOG_INFO ) << "org::xtreemfs::client::Proxy: using " << policy_function_name << " from " << policy_shared_library_path << ".";
            out_policy_function = policy_function;
            return true;
          }
          else
            return false;
        }
      };
    };
  };
};

#endif
