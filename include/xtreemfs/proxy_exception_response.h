// Copyright 2009 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef _XTREEMFS_PROXY_EXCEPTION_RESPONSE_H_
#define _XTREEMFS_PROXY_EXCEPTION_RESPONSE_H_

#include "yield.h"


namespace xtreemfs
{
  class ProxyExceptionResponse : public YIELD::concurrency::ExceptionResponse
  {
  public:
    virtual ~ProxyExceptionResponse() throw()
    { }

    virtual uint32_t get_error_code() const { return 0; }
    uint32_t get_platform_error_code() const;

    virtual const std::string& get_error_message() const
    { 
      return empty_error_message; 
    }

    virtual const std::string& get_stack_trace() const
    { 
      return empty_stack_trace; 
    }

    virtual void set_error_code( uint32_t ) { }

    // std::exception
    virtual const char* what() const throw()
    {
      if ( what_str.empty() )
      {
        std::ostringstream what_oss;
        what_oss << "errno = " << get_error_code();
        what_oss << ", strerror = " << get_error_message();
        if ( !get_stack_trace().empty() )
          what_oss << ", stack = " << get_stack_trace();
        const_cast<ProxyExceptionResponse*>( this )->what_str = what_oss.str();
      }

      return what_str.c_str();
    }

  protected:
    ProxyExceptionResponse()
    { }

    ProxyExceptionResponse( const char* )
    {
      DebugBreak();
    }

  private:
    std::string empty_error_message, empty_stack_trace, what_str;
  };
};

#define ORG_XTREEMFS_INTERFACES_EXCEPTION_RESPONSE_PARENT_CLASS ::xtreemfs::ProxyExceptionResponse

#endif
