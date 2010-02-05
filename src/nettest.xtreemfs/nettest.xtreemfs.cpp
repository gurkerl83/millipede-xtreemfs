// Copyright 2009-2010 Minor Gordon.
// This source comes from the XtreemFS project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#include "nettest_proxy.h"
#include "xtreemfs/main.h"

#include <iostream>


// Constants
#define RATE_DEFAULT 0
#define NUM_CALLS_DEFAULT 1


namespace nettest_xtreemfs
{
  class Main : public xtreemfs::Main
  {
  public:
    Main()
      : xtreemfs::Main
        ( 
          "nettest.xtreemfs", 
          "test the network connection to an XtreemFS server",
          "[oncrpc://]<host>:port"
        )
    {
      addOption
      (
        NETTEST_XTREEMFS_OPTION_NUM_CALLS, 
        "-n",
        "--num-calls",
        "number of RPCs to send"        
      );
      num_calls = NUM_CALLS_DEFAULT;

      addOption
      (
        NETTEST_XTREEMFS_OPTION_RATE, 
        "-r",
        "--rate",
        "rate to send RPCs in RPCs/s (default = 0 = as fast as the server will go)"
      );
      rate = RATE_DEFAULT;

      addOption
      (
        NETTEST_XTREEMFS_OPTION_RECV_BUFFER, 
        "--recv-buffer",
        NULL,
        "receive buffers with the given size (in K, power of 2, up to 2MB)"
      );

      addOption
      (
        NETTEST_XTREEMFS_OPTION_SEND_BUFFER, 
        "--send-buffer",
        NULL,
        "send buffers with the given size (in K)"
      );
    }

  private:
    enum
    {
      NETTEST_XTREEMFS_OPTION_NUM_CALLS = 20,
      NETTEST_XTREEMFS_OPTION_RATE = 21,
      NETTEST_XTREEMFS_OPTION_RECV_BUFFER = 22,
      NETTEST_XTREEMFS_OPTION_SEND_BUFFER = 23,
    };

    uint32_t num_calls;
    uint32_t rate;
    yidl::runtime::auto_Buffer recv_buffer;
    yidl::runtime::auto_Buffer send_buffer;
    YIELD::ipc::auto_URI uri;


    // YIELD::Main
    int _main( int, char** )
    {
      auto_NettestProxy 
        nettest_proxy
        ( 
          NettestProxy::create
          ( 
            *uri,
            NettestProxy::CONCURRENCY_LEVEL_DEFAULT,
            0,
            get_log(),
            get_operation_timeout(),
            NettestProxy::RECONNECT_TRIES_MAX_DEFAULT,
            get_proxy_ssl_context()
          ) 
        );         

      uint64_t sleep_after_each_call_ns;
      if ( rate > 0 )
      {
        sleep_after_each_call_ns 
          = static_cast<uint64_t>( ( 1.0 / static_cast<double>( rate ) )
            * static_cast<double>( NS_IN_S ) );
      }
      else
        sleep_after_each_call_ns = 0;

      uint64_t io_total_kb = 0;
      uint64_t rpc_time_total_ns = 0;
      YIELD::platform::Time start_wall_time;
 
      for ( uint32_t call_i = 0; call_i < num_calls; call_i++ )
      {
        YIELD::platform::Time rpc_time_start;

        if ( recv_buffer != NULL )
        {
          nettest_proxy->recv_buffer( recv_buffer->capacity(), recv_buffer );
          io_total_kb += recv_buffer->size() / 1024;
        }
        else if ( send_buffer != NULL )
        {
          nettest_proxy->send_buffer( send_buffer );
          io_total_kb += send_buffer->size() / 1024;
        }
        else
          nettest_proxy->nop();

        rpc_time_total_ns += YIELD::platform::Time() - rpc_time_start;    

        if ( sleep_after_each_call_ns > 0 )
          YIELD::platform::Thread::nanosleep( sleep_after_each_call_ns );
      }

      YIELD::platform::Time end_wall_time;
      uint64_t wall_time_total_ns = ( end_wall_time - start_wall_time );
      double wall_time_total_ms 
        = static_cast<double>( wall_time_total_ns ) 
          / static_cast<double>( NS_IN_MS );
      double wall_time_total_s 
        = static_cast<double>( wall_time_total_ns ) 
          / static_cast<double>( NS_IN_S );

      std::cout << "Elapsed wall time: " << 
         wall_time_total_ms << "ms" << std::endl;

      double rpc_time_total_ms 
        = static_cast<double>( rpc_time_total_ns ) 
          / static_cast<double>( NS_IN_MS );
      std::cout << "Elapsed time spent in RPCs: " <<
        rpc_time_total_ns << "ms" << std::endl;

      double kb_per_s 
        = static_cast<double>( io_total_kb ) / wall_time_total_s;        
      std::cout << "KB/s: " << kb_per_s << std::endl;

      return 0;
    }

    void parseFiles( int files_count, char** files )
    {
      if ( files_count >= 1 )
      {
        uri = parseURI( files[0] );
        if ( uri->get_port() == 0 )
          throw YIELD::platform::Exception( "must specify a port" );
      }
      else
        throw YIELD::platform::Exception( "must specify a URI" );
    }

    void parseOption( int id, char* arg )
    {
      switch ( id )
      {
        case NETTEST_XTREEMFS_OPTION_NUM_CALLS:
        {
          num_calls = atoi( arg );
          if ( num_calls == 0 )
            num_calls = NUM_CALLS_DEFAULT;
        }
        break;

        case NETTEST_XTREEMFS_OPTION_RATE:
        {
          rate = atoi( arg );
          if ( rate == 0 )
            rate = RATE_DEFAULT;
        }
        break;

        case NETTEST_XTREEMFS_OPTION_RECV_BUFFER:
        {
          uint32_t capacity_kb = atoi( arg );
          if ( capacity_kb > 0 )
          {
            // Round capacity up to the nearest power of 2
            uint32_t rounded_capacity_kb = 1;
            while ( rounded_capacity_kb < capacity_kb )
              rounded_capacity_kb <<= 1;

            recv_buffer
              = new yidl::runtime::HeapBuffer( rounded_capacity_kb * 1024 );
          }
        }
        break;

        case NETTEST_XTREEMFS_OPTION_SEND_BUFFER:
        {
          uint32_t capacity_kb = atoi( arg );
          if ( capacity_kb > 0 )
          {
            send_buffer 
              = new yidl::runtime::HeapBuffer( capacity_kb * 1024 );
            for ( uint32_t byte_i = 0; byte_i < capacity_kb * 1024; byte_i++ )
              send_buffer->put( "m", 1 );
          }
        }
        break;

        default:
        {
          xtreemfs::Main::parseOption( id, arg );
        }
        break;
      }
    }
  };
};

int main( int argc, char** argv )
{
  return nettest_xtreemfs::Main().main( argc, argv );
}
