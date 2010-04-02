// Copyright (c) 2010 Minor Gordon
// With original implementations and ideas contributed by Felix Hupfeld
// All rights reserved
// 
// This source file is part of the Yield project.
// It is licensed under the New BSD license:
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of the Yield project nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL Minor Gordon BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef _YIELD_CONCURRENCY_H_
#define _YIELD_CONCURRENCY_H_

#include "yield/platform.h"

#include <algorithm> // For std::sort
#include <queue>


// YIELD_CONCURRENCY_MG1_POLLING_TABLE_SIZE should be a Fibonnaci number:
// 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584
#define YIELD_CONCURRENCY_MG1_POLLING_TABLE_SIZE 144
#define YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX 64


namespace yield
{
  namespace concurrency
  {
    class Exception;
    class Request;
    class Response;
    class Stage;

    using yidl::runtime::atomic_cas;
    using yidl::runtime::atomic_t;
    using yidl::runtime::MarshallableObject;
    using yidl::runtime::MarshallableObjectFactory;
    using yidl::runtime::Marshaller;
    using yidl::runtime::Object;
    using yidl::runtime::RTTIObject;
    using yidl::runtime::Unmarshaller;

    using yield::platform::Mutex;
    using yield::platform::NOPLock;
    using yield::platform::ProcessorSet;
    using yield::platform::Semaphore;
    using yield::platform::Time;


    class Event : public MarshallableObject
    {
    public:
      virtual ~Event() { }

      virtual bool is_message() const { return false; }

      // Object
      Event& inc_ref() { return Object::inc_ref( *this ); }
    };


    class EventHandler : public RTTIObject
    {
    public:
      virtual ~EventHandler() { }

      virtual void handle( Event& event ) = 0;

      // Object
      EventHandler& inc_ref() { return Object::inc_ref( *this ); }

      // RTTIObject
      virtual uint32_t get_type_id() const { return 0; }
    };


    class EventQueue : public EventHandler
    {
    public:
      virtual ~EventQueue() { }

      virtual Event* dequeue() = 0;
      virtual Event* dequeue( const Time& timeout ) = 0;
      virtual bool enqueue( Event& ) = 0;
      virtual Event* try_dequeue() = 0;

      // RTTIObject
      const char* get_type_name() const { return "EventQueue"; }

      // EventHandler
      void handle( Event& event ) { enqueue( event ); }
    };


    class EventHandlerMux : public EventHandler
    {
    public:
      EventHandlerMux();
      ~EventHandlerMux();

      void add( EventHandler& event_handler );

      // RTTIObject
      const char* get_type_name() const { return "EventHandlerMux"; }

      // EventHandler
      void handle( Event& event );

    private:
      EventHandler** event_handlers;
      size_t event_handlers_len;
      size_t next_event_handler_i;
    };


    class Message : public Event
    {
    public:
      virtual ~Message() { }

      virtual bool is_request() const = 0;

      // Event
      bool is_message() const { return true; }

    protected:
      Message() { }
    };


    class MessageFactory : public MarshallableObjectFactory
    {
    public:
      virtual ~MessageFactory() { }

      virtual Exception* createException( uint32_t type_id )
      { 
        return NULL; 
      }

      virtual Exception* createException( const char* type_name )
      { 
        return createException( type_name, strlen( type_name ) );
      }

      virtual Exception*
      createException
      ( 
        const char* type_name,
        size_t type_name_len
      )
      {
        return NULL;
      }

      virtual Request* createRequest( uint32_t type_id ) { return NULL; }

      virtual Request* createRequest( const char* type_name )
      {
         return createRequest( type_name, strlen( type_name ) );
      }

      virtual Request*
      createRequest
      (
        const char* type_name,
        size_t type_name_len
      )
      {
        return NULL;
      }

      virtual Response* createResponse( uint32_t type_id ) { return NULL; }

      virtual Response* createResponse( const char* type_name )
      { 
        return createResponse( type_name, strlen( type_name ) );
      }
      
      virtual Response*
      createResponse
      (
        const char* type_name,
        size_t type_name_len
      )
      {
        return NULL;
      }
      
      // Object
      MessageFactory& inc_ref() { return Object::inc_ref( *this ); }
    };


    class MessageHandler : public EventHandler
    {
    public:
      virtual void handle( Message& message ) = 0;

      // EventHandler
      virtual void handle( Event& event );
    };


    template <class ElementType, uint32_t QueueLength>
    class NonBlockingFiniteQueue
    {
    public:
      NonBlockingFiniteQueue()
      {
        head = 0;
        tail = 1;

        for ( size_t element_i = 0; element_i < QueueLength+2; element_i++ )
          elements[element_i] = reinterpret_cast<ElementType>( 0 );

        elements[0] = reinterpret_cast<ElementType>( 1 );
      }

      ElementType dequeue()
      {
        atomic_t copied_head, try_pos;
        ElementType try_element;

        for ( ;; )
        {
          copied_head = head;
          try_pos = ( copied_head + 1 ) % ( QueueLength + 2 );
          try_element = reinterpret_cast<ElementType>( elements[try_pos] );

          while
          (
            try_element == reinterpret_cast<ElementType>( 0 ) ||
            try_element == reinterpret_cast<ElementType>( 1 )
          )
          {
            if ( copied_head != head )
              break;

            if ( try_pos == tail )
              return 0;

            try_pos = ( try_pos + 1 ) % ( QueueLength + 2 );

            try_element = reinterpret_cast<ElementType>( elements[try_pos] );
          }

          if ( copied_head != head )
            continue;

          if ( try_pos == tail )
          {
            atomic_cas( &tail, ( try_pos+1 ) % ( QueueLength+2 ), try_pos );
            continue;
          }

          if ( copied_head != head )
            continue;

          if
          (
            atomic_cas
            (
              // Memory
              reinterpret_cast<volatile atomic_t*>( &elements[try_pos] ),
              // New value
              (
                reinterpret_cast<atomic_t>( try_element) & POINTER_HIGH_BIT
              ) ? 1 : 0,
              // New value
              reinterpret_cast<atomic_t>( try_element )
            ) // Test against old value
            == reinterpret_cast<atomic_t>( try_element )
          )
          {
            if ( try_pos % 2 == 0 )
              atomic_cas( &head, try_pos, copied_head );

            return
              reinterpret_cast<ElementType>
              (
                (
                  reinterpret_cast<atomic_t>( try_element )
                  & POINTER_LOW_BITS
              ) << 1
            );
          }
        }
      }

      bool enqueue( ElementType element )
      {
#ifdef _DEBUG
        if ( reinterpret_cast<atomic_t>( element ) & 0x1 )
          DebugBreak();
#endif

        element 
          = reinterpret_cast<ElementType>
            (
              reinterpret_cast<atomic_t>( element ) >> 1
            );

#ifdef _DEBUG
        if ( reinterpret_cast<atomic_t>( element ) & POINTER_HIGH_BIT )
          DebugBreak();
#endif

        atomic_t copied_tail, last_try_pos, try_pos; // te, ate, temp
        ElementType try_element;

        for ( ;; )
        {
          copied_tail = tail;
          last_try_pos = copied_tail;
          try_element = reinterpret_cast<ElementType>( elements[last_try_pos] );
          try_pos = ( last_try_pos + 1 ) % ( QueueLength + 2 );

          while
          (
            try_element != reinterpret_cast<ElementType>( 0 ) 
            &&
            try_element != reinterpret_cast<ElementType>( 1 )
          )
          {
            if ( copied_tail != tail )
              break;

            if ( try_pos == head )
              break;

            try_element = reinterpret_cast<ElementType>( elements[try_pos] );
            last_try_pos = try_pos;
            try_pos = ( last_try_pos + 1 ) % ( QueueLength + 2 );
          }

          if ( copied_tail != tail ) // Someone changed tail
            continue;                // while we were looping

          if ( try_pos == head )
          {
            last_try_pos = ( try_pos + 1 ) % ( QueueLength + 2 );
            try_element
              = reinterpret_cast<ElementType>( elements[last_try_pos] );

            if 
            ( 
              try_element != reinterpret_cast<ElementType>( 0 )
              &&
              try_element != reinterpret_cast<ElementType>( 1 ) 
            )
              return false; // Queue is full

            atomic_cas( &head, last_try_pos, try_pos );
            continue;
          }

          if ( copied_tail != tail )
            continue;

          // diff next line
          if
          (
            atomic_cas
            (
              // Memory
              reinterpret_cast<volatile atomic_t*>
              (
                &elements[last_try_pos]
              ),

              // New value
              try_element == reinterpret_cast<ElementType>( 1 ) ?
                ( reinterpret_cast<atomic_t>( element )
                  | POINTER_HIGH_BIT ) :
                reinterpret_cast<atomic_t>( element ),

              // Old value
              reinterpret_cast<atomic_t>( try_element )

            ) // Test against old value
            == reinterpret_cast<atomic_t>( try_element )
          )
          {
            if ( try_pos % 2 == 0 )
              atomic_cas( &tail, try_pos, copied_tail );

            return true;
          }
        }
      }

    private:
      volatile ElementType elements[QueueLength+2]; // extra 2 for sentinels
      volatile atomic_t head, tail;

#if defined(__LLP64__) || defined(__LP64__)
      const static atomic_t POINTER_HIGH_BIT = 0x8000000000000000;
      const static atomic_t POINTER_LOW_BITS = 0x7fffffffffffffff;
#else
      const static atomic_t POINTER_HIGH_BIT = 0x80000000;
      const static atomic_t POINTER_LOW_BITS = 0x7fffffff;
#endif
    };


    template <class ElementType, uint32_t QueueLength>
    class SynchronizedNonBlockingFiniteQueue
      : private NonBlockingFiniteQueue<ElementType, QueueLength>
    {
    public:
      ElementType dequeue()
      {
        ElementType element =
          NonBlockingFiniteQueue<ElementType, QueueLength>::dequeue();

        while ( element == 0 )
        {
          signal.acquire();
          element = NonBlockingFiniteQueue<ElementType, QueueLength>::dequeue();
        }

        return element;
      }

      ElementType dequeue( const Time& timeout )
      {
        ElementType element
          = NonBlockingFiniteQueue<ElementType, QueueLength>::dequeue();

        if ( element != 0 )
          return element;
        else
        {
          signal.acquire( timeout );
          return NonBlockingFiniteQueue<ElementType, QueueLength>::dequeue();
        }
      }

      bool enqueue( ElementType element )
      {
        bool enqueued =
          NonBlockingFiniteQueue<ElementType, QueueLength>::enqueue( element );
        signal.release();
        return enqueued;
      }

      ElementType try_dequeue()
      {
        return NonBlockingFiniteQueue<ElementType, QueueLength>::dequeue();
      }

    private:
      Semaphore signal;
    };


    class NonBlockingEventQueue
      : public EventQueue,
        private SynchronizedNonBlockingFiniteQueue<Event*,1024>
    {
    public:
      // EventQueue
      Event* dequeue()
      {
        return SynchronizedNonBlockingFiniteQueue<Event*,1024>::dequeue();
      }

      Event* dequeue( const Time& timeout )
      {
        return SynchronizedNonBlockingFiniteQueue<Event*,1024>
                 ::dequeue( timeout );
      }

      bool enqueue( Event& event )
      {
        return SynchronizedNonBlockingFiniteQueue<Event*,1024>::enqueue
               ( 
                 &event 
               );
      }

      Event* try_dequeue()
      {
        return SynchronizedNonBlockingFiniteQueue<Event*,1024>::try_dequeue();
      }
    };


    class Request : public Message
    {
    public:
      Request();
      virtual ~Request();

      EventHandler* get_response_handler() const;
      virtual void respond( Response& response );
      void set_response_handler( EventHandler* response_handler );

      // Object
      Request& inc_ref() { return Object::inc_ref( *this ); }

      // Message
      bool is_request() const { return true; }

    private:
      EventHandler* response_handler;
    };


    class RequestHandler : public MessageHandler
    {
    public:
      virtual void handle( Request& request ) = 0;

      // MessageHandler
      virtual void handle( Message& message );
    };

    
    class Response : public Message
    {
    public:
      Response() { }
      virtual ~Response() { }

      virtual bool is_exception() const { return false; }

      // Object
      Response& inc_ref() { return Object::inc_ref( *this ); }

      // Message
      bool is_request() const { return false; }
    };


    class Exception : public Response, public yield::platform::Exception
    {
    public:
      Exception()
      { }

      Exception( uint32_t error_code )
        : yield::platform::Exception( error_code )
      { }

      Exception( const char* error_message )
        : yield::platform::Exception( error_message )
      { }

      Exception( const string& error_message )
        : yield::platform::Exception( error_message )
      { }

      Exception( uint32_t error_code, const char* error_message )
        : yield::platform::Exception( error_code, error_message )
      { }

      Exception( uint32_t error_code, const string& error_message )
        : yield::platform::Exception( error_code, error_message )
      { }

      Exception( const Exception& other )
        : yield::platform::Exception( other )
      { }

      virtual ~Exception() throw() 
      { }

      virtual Exception& clone() const { return *new Exception( *this ); }
      virtual void throwStackClone() const { throw Exception( *this ); }

      // Object
      Exception& inc_ref() { return Object::inc_ref( *this ); }

      // RTTIObject
      virtual uint32_t get_type_id() const { return 0; }
      virtual const char* get_type_name() const { return "Exception"; }

      // MarshallableObject
      virtual void marshal( Marshaller& ) const { }
      virtual void unmarshal( Unmarshaller& ) { }

      // Response
      bool is_exception() const { return true; }
    };


    class ResponseHandler : public MessageHandler
    {
    public:
      virtual void handle( Response& response ) = 0;

      // MessageHandler
      void handle( Message& message );
    };


    template <class ElementType>
    class SynchronizedSTLQueue : private std::queue<ElementType>
    {
    public:
      ElementType dequeue()
      {
        for ( ;; )
        {
          signal.acquire();
          lock.acquire();
          if ( std::queue<ElementType>::size() > 0 )
          {
            ElementType element = std::queue<ElementType>::front();
            std::queue<ElementType>::pop();
            lock.release();
            return element;
          }
          else
            lock.release();
        }
      }

      ElementType dequeue( const Time& timeout )
      {
        Time timeout_left( timeout );

        for ( ;; )
        {
          Time start_time;

          if ( signal.acquire( timeout_left ) )
          {
            if ( lock.try_acquire() )
            {
              if ( std::queue<ElementType>::size() > 0 )
              {
                ElementType element = std::queue<ElementType>::front();
                std::queue<ElementType>::pop();
                lock.release();
                return element;
              }
              else
                lock.release();
            }
          }

          Time elapsed_time; elapsed_time -= start_time;
          if ( elapsed_time < timeout_left )
            timeout_left -= elapsed_time;
          else
            return NULL;
        }
      }

      bool enqueue( ElementType element )
      {
        lock.acquire();
        std::queue<ElementType>::push( element );
        lock.release();
        signal.release();
        return true;
      }

      ElementType try_dequeue()
      {
        if ( lock.try_acquire() )
        {
          if ( std::queue<ElementType>::size() > 0 )
          {
            ElementType element = std::queue<ElementType>::front();
            std::queue<ElementType>::pop();
            lock.release();
            return element;
          }
          else
            lock.release();
        }

        return NULL;
      }

    private:
      Mutex lock;
      Semaphore signal;
    };


    class STLEventQueue
      : public EventQueue,
        private SynchronizedSTLQueue<Event*>
    {
    public:
      // Object
      STLEventQueue& inc_ref() { return Object::inc_ref( *this ); }

      // EventQueue
      Event* dequeue()
      {
        return SynchronizedSTLQueue<Event*>::dequeue();
      }

      Event* dequeue( const Time& timeout )
      {
        return SynchronizedSTLQueue<Event*>::dequeue( timeout );
      }

      bool enqueue( Event& event )
      {
        return SynchronizedSTLQueue<Event*>::enqueue( &event );
      }

      Event* try_dequeue()
      {
        return SynchronizedSTLQueue<Event*>::try_dequeue();
      }
    };


    class ThreadLocalEventQueue : public EventQueue
    {
    public:
      ThreadLocalEventQueue();
      ~ThreadLocalEventQueue();

      // EventQueue
      Event* dequeue();
      Event* dequeue( const Time& timeout );
      bool enqueue( Event& );      
      Event* try_dequeue();

    private:
      class EventStack;

      unsigned long tls_key;
      vector<EventStack*> event_stacks;
      EventStack* getEventStack();

      SynchronizedSTLQueue<Event*> all_processor_event_queue;
    };


    template <class ResponseType>
    class ResponseQueue
      : public ResponseHandler,
        private SynchronizedSTLQueue<Response*>
    {
    public:
      ResponseType& dequeue()
      {
        Response* response = SynchronizedSTLQueue<Response*>::dequeue();

        if ( response->get_type_id() == ResponseType::TYPE_ID )
          return static_cast<ResponseType&>( *response );
        else if ( response->is_exception() )
        {
          try
          {
            static_cast<Exception*>( response )->throwStackClone();
            // Eliminate compiler warnings about control paths
            return static_cast<ResponseType&>( *response );
          }
          catch ( Exception& )
          {
            Response::dec_ref( *response );
            throw;
          }
        }
        else
          throw Exception( "ResponseQueue: dequeued unexpected Response type" );
      }

      ResponseType& dequeue( const Time& timeout )      
      {
        Response* response 
          = SynchronizedSTLQueue<Response*>::dequeue( timeout );

        if ( response != NULL )
        {
          if ( response->get_type_id() == ResponseType::TYPE_ID )
            return static_cast<ResponseType&>( *response );
          else if ( response->is_exception() )
          {
            try
            {
              static_cast<Exception*>( response )->throwStackClone();
              // Eliminate compiler warnings about control paths
              return static_cast<ResponseType&>( *response );
            }
            catch ( Exception& )
            {
              Response::dec_ref( *response );
              throw;
            }
          }
          else
            throw Exception( "ResponseQueue: dequeued unexpected Response type" );
        }
        else
          throw Exception( "ResponseQueue::dequeue: timed out" );
      }

      // RTTIObject
      virtual const char* get_type_name() const { return "ResponseQueue"; }

      // ResponseHandler
      void handle( Response& response )
      {
        SynchronizedSTLQueue<Response*>::enqueue( &response );
      }
    };


    template<typename SampleType, size_t ArraySize, class LockType = NOPLock>
    class Sampler
    {
    public:
      Sampler()
      {
        memset( samples, 0, sizeof( samples ) );
        samples_pos = samples_count = 0;
        min = static_cast<SampleType>( ULONG_MAX ); max = 0; total = 0;
      }

      void clear()
      {
        lock.acquire();
        samples_count = 0;
        lock.release();
      }

      SampleType get_max() const
      {
        return max;
      }

      SampleType get_mean()
      {
        lock.acquire();
        SampleType mean;

        if ( samples_count > 0 )
          mean = static_cast<SampleType>
                 (
                   static_cast<double>( total ) /
                   static_cast<double>( samples_count )
                 );
        else
          mean = 0;

        lock.release();
        return mean;
      }

      SampleType get_median()
      {
        lock.acquire();
        SampleType median;

        if ( samples_count > 0 )
        {
          std::sort( samples, samples + samples_count );
          size_t sc_div_2 = samples_count / 2;
          if ( samples_count % 2 == 1 )
            median = samples[sc_div_2];
          else
          {
            SampleType median_temp = samples[sc_div_2] + samples[sc_div_2-1];
            if ( median_temp > 0 )
              median = static_cast<SampleType>
                       (
                         static_cast<double>( median_temp ) / 2.0
                       );
            else
              median = 0;
          }
        }
        else
          median = 0;

        lock.release();
        return median;
      }

      SampleType get_min() const
      {
        return min;
      }

      SampleType get_percentile( double percentile )
      {
        if ( percentile > 0 && percentile < 100 )
        {
          lock.acquire();
          SampleType value;

          if ( samples_count > 0 )
          {
            std::sort( samples, samples + samples_count );
            value =
              samples[static_cast<size_t>( percentile *
                static_cast<double>( samples_count ) )];
          }
          else
            value = 0;

          lock.release();
          return value;
        }
        else
          return 0;
      }

      uint32_t get_samples_count() const
      {
        return samples_count;
      }

      void set_next_sample( SampleType sample )
      {
        if ( lock.try_acquire() )
        {
          samples[samples_pos] = sample;
          samples_pos = ( samples_pos + 1 ) % ArraySize;
          if ( samples_count < ArraySize ) samples_count++;

          if ( sample < min )
            min = sample;
          if ( sample > max )
            max = sample;
          total += sample;

          lock.release();
        }
      }

    protected:
      SampleType samples[ArraySize+1], min, max; SampleType total;
      uint32_t samples_pos, samples_count;
      LockType lock;
    };


    class Stage : public EventHandler
    {
    public:
      class StartupEvent : public Event
      {
      public:
        StartupEvent( Stage& stage )
          : stage( stage.inc_ref() )
        { }

        ~StartupEvent()
        {
          Stage::dec_ref( stage );
        }

        Stage& get_stage() { return stage; }

        // RTTIObject
        YIDL_RUNTIME_RTTI_OBJECT_PROTOTYPES( Stage::StartupEvent, 104 );

        // MarshallableObject
        void marshal( Marshaller& ) const { }
        void unmarshal( Unmarshaller& ) { }

      private:
        Stage& stage;
      };


      class ShutdownEvent : public Event
      {
      public:
        // RTTIObject
        YIDL_RUNTIME_RTTI_OBJECT_PROTOTYPES( Stage::ShutdownEvent, 105 );

        // MarshallableObject
        void marshal( Marshaller& ) const { }
        void unmarshal( Unmarshaller& ) { }
      };


      virtual ~Stage();

      double get_arrival_rate_s() const { return arrival_rate_s; }
      double get_rho() const { return rho; }
      double get_service_rate_s() const { return service_rate_s; }
      uint8_t get_stage_id() const { return id; }
      virtual EventHandler& get_event_handler() = 0;
      virtual bool visit() = 0;
      virtual bool visit( const Time& timeout ) = 0;
      virtual void visit( Event& event ) = 0;

      // Object
      Stage& inc_ref() { return Object::inc_ref( *this ); }

    protected:
      Stage();

      Sampler<uint64_t, 1024, Mutex> event_processing_time_sampler;
      uint32_t event_queue_length, event_queue_arrival_count;
  #ifdef YIELD_PLATFORM_HAVE_PERFORMANCE_COUNTERS
      PerformanceCounterSet& performance_counters;
      uint64_t performance_counter_totals[2];
  #endif

    private:
      class StatisticsTimer;

      uint8_t id;
      double arrival_rate_s, rho, service_rate_s;

      friend class StageGroup;
      void set_stage_id( uint8_t stage_id ) { this->id = stage_id; }
    };


    template <class EventHandlerType, class EventQueueType, class LockType>
    class StageImpl : public Stage
    {
    public:
      StageImpl
      (
        EventHandlerType& event_handler,
        EventQueueType& event_queue
      )
        : event_handler( event_handler ),
          event_queue( event_queue )
      { }
      
      virtual ~StageImpl()
      {
        EventHandlerType::dec_ref( event_handler );
        EventQueueType::dec_ref( event_queue );
      }

      const char* get_type_name() const { return "StageImpl"; }

      // EventHandler
      void handle( Event& event )
      {
        ++event_queue_length;
        ++event_queue_arrival_count;

        if ( event_queue.enqueue( event ) )
          return;
        else
        {
          cerr << event_handler.get_type_name() << 
            ": event queue full, stopping.";
          DebugBreak();
        }
      }

      EventHandler& get_event_handler() { return event_handler; }

      bool visit()
      {
        lock.acquire();

        Event* event = event_queue.dequeue();

        --event_queue_length;
        _handle( *event );

        for ( ;; )
        {
          event = event_queue.try_dequeue();
          if ( event != NULL )
          {
            --event_queue_length;
            handle( *event );
          }
          else
            break;
        }

        lock.release();

        return true;
      }

      bool visit( const Time& timeout )
      {
        if ( lock.try_acquire() )
        {
          Event* event = event_queue.dequeue( timeout );
          if ( event != NULL )
          {
            --event_queue_length;
            _handle( *event );

            for ( ;; )
            {
              event = event_queue.try_dequeue();
              if ( event != NULL )
              {
                --event_queue_length;
                _handle( *event );
              }
              else
                break;
            }

            lock.release();

            return true;
          }
          else
          {
            lock.release();
            return false;
          }
        }
        else
          return false;
      }

      void visit( Event& event )
      {
        --event_queue_length;
        lock.acquire();
        _handle( event );
        lock.release();
      }

    private:
      EventHandlerType& event_handler;
      EventQueueType& event_queue;

      LockType lock;

      void _handle( Event& event )
      {
        Time start_time;

  #ifdef YIELD_PLATFORM_HAVE_PERFORMANCE_COUNTERS
        performance_counters->startCounting();
  #endif

        event_handler.handle( event );

  #ifdef YIELD_PLATFORM_HAVE_PERFORMANCE_COUNTERS
        uint64_t performance_counter_counts[2];
        performance_counters->stopCounting( performance_counter_counts );
        performance_counter_totals[0] += performance_counter_counts[0];
        performance_counter_totals[1] += performance_counter_counts[1];
  #endif

        Time event_processing_time;
        event_processing_time -= start_time;
        if ( event_processing_time < 10.0 )
        {
          event_processing_time_sampler.
            set_next_sample( event_processing_time );
        }
      }
    };


    class StageGroup : public Object
    {
    public:
      virtual ~StageGroup();

      // createStage steals the event_handler reference passed to it,
      // to allow createStage( *new EventHandlerType )
      template <class EventHandlerType>
      Stage* createStage( EventHandlerType& event_handler )
      {
        return createStage( static_cast<EventHandler&>( event_handler ) );
      }

      template <class EventHandlerType>
      Stage*
      createStage
      (
        EventHandlerType& event_handler,
        int16_t thread_count
      )
      {
        return createStage
        (
          static_cast<EventHandler&>( event_handler ),
          thread_count
        );
      }

      virtual Stage* 
      createStage
      (
        EventHandler& event_handler,
        int16_t thread_count = 1
      ) = 0;

      Stage** get_stages() { return &stages[0]; }

    protected:
      StageGroup();      

      void addStage( Stage* stage );

    private:
      Stage* stages[YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX];
    };


    template <class StageGroupType> // CRTP
    class StageGroupImpl : public StageGroup
    {
    public:
      virtual ~StageGroupImpl() { }

      template <class EventHandlerType>
      Stage* createStage
      (
        EventHandlerType& event_handler
      )
      {
        return static_cast<StageGroupType*>( this )
          ->createStage<EventHandlerType, EventQueue>( event_handler, 1 );
      }

      template <class EventHandlerType>
      Stage* createStage
      (
        EventHandlerType& event_handler,
        int16_t thread_count
      )
      {
        return static_cast<StageGroupType*>( this )
          ->createStage<EventHandlerType>( event_handler, thread_count );
      }

      // StageGroup
      Stage* createStage
      (
        EventHandler& event_handler,
        int16_t thread_count = 1
      )
      {
        return createStage<EventHandler>( event_handler, thread_count );
      }

    protected:
      StageGroupImpl() { }      
    };


    class ColorStageGroup : public StageGroupImpl<ColorStageGroup>
    {
    public:
      ColorStageGroup
      (
        const char* name = "Main stage group",
        uint16_t start_logical_processor_i = 0,
        int16_t thread_count = -1
      );

      ~ColorStageGroup();

      template <class EventHandlerType>
      Stage* createStage
      (
        EventHandlerType& event_handler,
        int16_t thread_count
      )
      {
        Stage* stage;

        if ( thread_count == 1 )
        {
          stage
            = new StageImpl<EventHandlerType, STLEventQueue, NOPLock>
                  ( 
                    event_handler, 
                    event_queue->inc_ref() 
                  );
        }
        else
        {
          stage
            = new StageImpl<EventHandlerType, STLEventQueue, Mutex>
                  ( 
                    event_handler, 
                    event_queue->inc_ref() 
                  );
        }

        // TODO: check flags before sending this
        //event_handler->handle( *new Stage::StartupEvent( stage ) );

        this->addStage( stage );

        return stage;
      }

    private:
      STLEventQueue* event_queue;

      class Thread;
      vector<Thread*> threads;
    };


    class VisitPolicy
    {
    protected:
      VisitPolicy( Stage** stages )
        : stages( stages )
      { }

      Stage** stages;
    };


    template <class VisitPolicyType>
    class PollingStageGroup
      : public StageGroupImpl< PollingStageGroup<VisitPolicyType> >
    {
    public:
      PollingStageGroup
      (
        const char* name = "Main stage group",
        uint16_t start_logical_processor_i = 0,
        int16_t thread_count = -1,
        bool use_thread_local_event_queues = false
      );

      ~PollingStageGroup();

      template <class EventHandlerType>
      Stage* createStage
      (
        EventHandlerType& event_handler,
        int16_t thread_count
      )
      {
        Stage* stage;

        if ( use_thread_local_event_queues )
        {
          if ( thread_count == 1 )
          {
            stage
              = new StageImpl<EventHandlerType, ThreadLocalEventQueue, Mutex>                      
                    ( 
                      event_handler, 
                      *new ThreadLocalEventQueue 
                    );
          }
          else
          {
            stage
              = new StageImpl<EventHandlerType, ThreadLocalEventQueue, NOPLock>
                    ( 
                      event_handler, 
                      *new ThreadLocalEventQueue 
                    );
          }
        }
        else
        {
          if ( thread_count == 1 )
          {
            stage
              = new StageImpl<EventHandlerType, STLEventQueue, Mutex>
                    ( 
                      event_handler, 
                      *new STLEventQueue 
                    );
          }
          else
          {
            stage
              = new StageImpl<EventHandlerType, STLEventQueue, NOPLock>
                    ( 
                      event_handler, 
                      *new STLEventQueue 
                    );
          }
        }

        // TODO: check flags before sending this
        //event_handler->handle( *new Stage::StartupEvent( stage ) );

        this->addStage( stage );

        return stage;
      }

    private:
      bool use_thread_local_event_queues;

      class Thread;
      vector<Thread*> threads;
    };


    class DBRVisitPolicy : public VisitPolicy
    {
    public:
      DBRVisitPolicy( Stage** stages )
        : VisitPolicy( stages )
      {
        next_stage_i = YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX;
        memset( sorted_stages, 0, sizeof( sorted_stages ) );
      }

      // VisitPolicy
      inline Stage* getNextStageToVisit( bool last_visit_was_successful )
      {
        if ( last_visit_was_successful )
        {
          next_stage_i = 0;
          return sorted_stages[0];
        }
        else if ( next_stage_i < YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX )
          return sorted_stages[next_stage_i++];
        else
        {
          memcpy_s
          (
            sorted_stages,
            sizeof( sorted_stages ),
            stages,
            sizeof( sorted_stages )
          );

          std::sort
          (
            &sorted_stages[0],
            &sorted_stages[YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX-1],
            compare_stages()
          );

          next_stage_i = 0;

          return sorted_stages[0];
        }
      }

    private:
      uint8_t next_stage_i;
      Stage* sorted_stages[YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX];

      struct compare_stages : public std::binary_function<Stage*, Stage*, bool>
      {
        bool operator()( Stage* left, Stage* right )
        {
          if ( left != NULL )
          {
            if ( right != NULL )
              return left->get_service_rate_s() < right->get_service_rate_s();
            else
              return true;
          }
          else
            return false;
        }
      };
    };


    class MG1VisitPolicy : public VisitPolicy
    {
    public:
      MG1VisitPolicy( Stage** stages );

      // VisitPolicy
      inline Stage* getNextStageToVisit( bool )
      {
        if ( polling_table_pos < YIELD_CONCURRENCY_MG1_POLLING_TABLE_SIZE )
          return stages[polling_table[polling_table_pos++]];
        else
        {
          populatePollingTable();
          polling_table_pos = 0;
          return stages[polling_table[polling_table_pos++]];
        }
      }

    private:
      uint8_t polling_table[YIELD_CONCURRENCY_MG1_POLLING_TABLE_SIZE];
      uint32_t polling_table_pos;
      uint32_t golden_ratio_circle[YIELD_CONCURRENCY_MG1_POLLING_TABLE_SIZE];
      // These are only used in populating the polling table,
      // but we have to keep the values to use in smoothing
      double last_rhos[YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX];

      bool populatePollingTable();
    };


    class SRPTVisitPolicy : public VisitPolicy
    {
    public:
      SRPTVisitPolicy( Stage** stages ) : VisitPolicy( stages )
      {
        next_stage_i = 0;
      }

      // VisitPolicy
      inline Stage* getNextStageToVisit( bool last_visit_was_successful )
      {
        if ( last_visit_was_successful )
          next_stage_i = 0;
        else
          next_stage_i = ( next_stage_i + 1 ) % YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX;

        return stages[next_stage_i];
      }

    private:
      unsigned char next_stage_i;
    };


    class WavefrontVisitPolicy : public VisitPolicy
    {
    public:
      WavefrontVisitPolicy( Stage** stages )
        : VisitPolicy( stages )
      {
        forward = true;
        next_stage_i = 0;
      }

      // VisitPolicy
      inline Stage* getNextStageToVisit( bool )
      {
        if ( forward )
        {
          if ( next_stage_i < YIELD_CONCURRENCY_STAGES_PER_GROUP_MAX - 1 )
            ++next_stage_i;
          else
            forward = false;
        }
        else
        {
          if ( next_stage_i > 0 )
            --next_stage_i;
          else
            forward = true;
        }

        return stages[next_stage_i];
      }

    private:
      bool forward;
      unsigned char next_stage_i;
    };


    class SEDAStageGroup : public StageGroupImpl<SEDAStageGroup>
    {
    public:
      SEDAStageGroup() { }

      template <class EventHandlerType>
      Stage* createStage
      (
        EventHandlerType& event_handler,
        int16_t thread_count
      )
      {
        if ( thread_count <= 0 )
          thread_count = ProcessorSet::getOnlinePhysicalProcessorCount();

        Stage* stage;
        if ( thread_count == 1 )
        {
          stage
            = new StageImpl<EventHandlerType, STLEventQueue, Mutex>
                  ( 
                    event_handler, 
                    *new STLEventQueue 
                  );
        }
        else
        {
          stage
            = new StageImpl<EventHandlerType, STLEventQueue, NOPLock>
                  (
                    event_handler, 
                    *new STLEventQueue 
                  );
        }

        // TODO: check flags before sending this
        //event_handler->handle( *new Stage::StartupEvent( stage ) );

        this->addStage( stage );

        startThreads( *stage, thread_count );

        return stage;
      }

    protected:
      virtual ~SEDAStageGroup();

    private:
      class Thread;
      vector<Thread*> threads;
      void startThreads( Stage& stage, int16_t thread_count );
    };
  };
};


#endif
