// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "org/xtreemfs/client/proxy_exception_response.h"
using namespace org::xtreemfs::client;

#ifdef _WIN32
#include "yield/platform/windows.h"
#else
#include <errno.h>
#endif


uint32_t ProxyExceptionResponse::get_platform_error_code() const
{
  uint32_t error_code = get_error_code();

  switch ( error_code )
  {
#ifdef _WIN32
    case EACCES: return ERROR_ACCESS_DENIED;
    case EEXIST: return ERROR_ALREADY_EXISTS;
    case EINVAL: return ERROR_INVALID_PARAMETER;
    case ENOENT: return ERROR_FILE_NOT_FOUND;
    case WSAETIMEDOUT: return ERROR_NETWORK_BUSY;
#endif
    case 0: YIELD::DebugBreak(); return 0;
    default: return error_code;
  }
}
