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

#include <map>
#include <queue>


#define YIELD_STAGES_PER_GROUP_MAX 64
// YIELD_CONCURRENCY_MG1_POLLING_TABLE_SIZE should be a Fibonnaci number:
// 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584
#define YIELD_CONCURRENCY_MG1_POLLING_TABLE_SIZE 144


namespace YIELD
{
  namespace concurrency
  {
    class ExceptionResponse;
    class Request;
    class Response;
    class Stage;


    class Event : public yidl::runtime::Struct
    {
    public:
      Event()
        : next_stage( NULL )
      { }

      Stage* get_next_stage() const
      {
          return next_stage;
      }

      void set_next_stage( Stage* next_stage )
      {
          this->next_stage = next_stage;
      }

      // yidl::runtime::Object
      Event& incRef() { return Object::incRef( *this ); }

    protected:
      virtual ~Event() { }

    private:
      Stage* next_stage;
    };

    typedef yidl::runtime::auto_Object<Event> auto_Event;


    class EventTarget : public yidl::runtime::Object
    {
    public:
      virtual void send( Event& ) = 0;

      // yidl::runtime::Object
      EventTarget& incRef() { return yidl::runtime::Object::incRef( *this ); }

    protected:
      EventTarget() { }
      virtual ~EventTarget() { }
    };

    typedef yidl::runtime::auto_Object<EventTarget> auto_EventTarget;


    class EventTargetMux : public EventTarget
    {
    public:
      EventTargetMux();

      void addEventTarget( auto_EventTarget event_target );

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( EventTargetMux, 0 );

      // EventTarget
      void send( Event& );

    private:
      ~EventTargetMux();

      EventTarget** event_targets;
      size_t event_targets_len;
      size_t next_event_target_i;
    };

    typedef yidl::runtime::auto_Object<EventTargetMux> auto_EventTargetMux;


    class EventHandler : public EventTarget
    {
    public:
      virtual void handleEvent( Event& ) = 0;
      virtual void handleUnknownEvent( Event& );
      virtual bool isThreadSafe() const { return false; }

      void set_redirect_event_target( EventTarget* redirect_event_target );

      // yidl::runtime::Object
      EventHandler& incRef() { return yidl::runtime::Object::incRef( *this ); }

      // EventTarget
      void send( Event& );

    protected:
      EventHandler();
      virtual ~EventHandler() { }

    private:
      YIELD::platform::Mutex handleEvent_lock;
      EventTarget* redirect_event_target;
    };

    typedef yidl::runtime::auto_Object<EventHandler> auto_EventHandler;


    class EventQueue : public EventTarget
    {
    public:
      virtual Event* dequeue() = 0;
      virtual bool enqueue( Event& ) = 0;
      virtual Event* timed_dequeue( const YIELD::platform::Time& timeout ) = 0;
      virtual Event* try_dequeue() = 0;

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( EventQueue, 0 );

      // EventTarget
      void send( Event& event )
      {
        enqueue( event );
      }
    };

    typedef yidl::runtime::auto_Object<EventQueue> auto_EventQueue;


    class NonBlockingEventQueue
      : public EventQueue,
        private YIELD::platform::
                  SynchronizedNonBlockingFiniteQueue<Event*, 1024>
    {
    public:
      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( NonBlockingEventQueue, 0 );

      // EventQueue
      Event* dequeue()
      {
        return YIELD::platform::
                SynchronizedNonBlockingFiniteQueue<Event*, 1024>::
                  dequeue();
      }

      bool enqueue( Event& ev )
      {
        return YIELD::platform::
                SynchronizedNonBlockingFiniteQueue<Event*, 1024>::
                  enqueue( &ev );
      }

      Event* timed_dequeue( const YIELD::platform::Time& timeout )
      {
        return YIELD::platform::
                SynchronizedNonBlockingFiniteQueue<Event*, 1024>::
                  timed_dequeue( timeout );
      }

      Event* try_dequeue()
      {
        return YIELD::platform::
                 SynchronizedNonBlockingFiniteQueue<Event*, 1024>::
                   try_dequeue();
      }
    };

    typedef yidl::runtime::auto_Object<NonBlockingEventQueue>
      auto_NonBlockingEventQueue;


    class STLEventQueue
      : public EventQueue,
        private YIELD::platform::SynchronizedSTLQueue<Event*>
    {
    public:
      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( STLEventQueue, 0 );

      // EventQueue
      Event* dequeue()
      {
        return YIELD::platform::SynchronizedSTLQueue<Event*>::dequeue();
      }

      bool enqueue( Event& ev )
      {
        return YIELD::platform::SynchronizedSTLQueue<Event*>::enqueue( &ev );
      }

      Event* timed_dequeue( const YIELD::platform::Time& timeout )
      {
        return YIELD::platform::
                 SynchronizedSTLQueue<Event*>::
                   timed_dequeue( timeout );
      }

      Event* try_dequeue()
      {
        return YIELD::platform::SynchronizedSTLQueue<Event*>::try_dequeue();
      }
    };

    typedef yidl::runtime::auto_Object<STLEventQueue> auto_STLEventQueue;


    class ThreadLocalEventQueue : public EventQueue
    {
    public:
      ThreadLocalEventQueue();
      ~ThreadLocalEventQueue();

      // EventQueue
      Event* dequeue();
      bool enqueue( Event& );
      Event* timed_dequeue( const YIELD::platform::Time& timeout );
      Event* try_dequeue();

    private:
      class EventStack;

      unsigned long tls_key;
      std::vector<EventStack*> event_stacks;
      EventStack* getEventStack();

      YIELD::platform::SynchronizedSTLQueue<Event*> all_processor_event_queue;
    };

    typedef yidl::runtime::auto_Object<ThreadLocalEventQueue>
      auto_ThreadLocalEventQueue;


    class Interface : public EventHandler
    {
    public:
      // Casts an Object to a Request if the request belongs to the interface
      virtual Request* checkRequest( yidl::runtime::Object& request ) = 0;

      // Casts an Object to a Response if the request belongs to the interface
      virtual Response* checkResponse( yidl::runtime::Object& response ) = 0;

      virtual yidl::runtime::auto_Object<Request>
        createRequest( uint32_t tag ) = 0;

      virtual yidl::runtime::auto_Object<Response>
        createResponse( uint32_t tag ) = 0;

      virtual yidl::runtime::auto_Object<ExceptionResponse>
        createExceptionResponse( uint32_t tag ) = 0;
    };

    typedef yidl::runtime::auto_Object<Interface> auto_Interface;


    class Request : public Event
    {
    public:
      virtual yidl::runtime::auto_Object<Response> createResponse() = 0;

      auto_EventTarget get_response_target() const;
      virtual void respond( Response& response );
      void set_response_target( auto_EventTarget response_target );

      // yidl::runtime::Object
      Request& incRef() { return Object::incRef( *this ); }

    protected:
      Request() { }
      virtual ~Request() { }

    private:
      auto_EventTarget response_target;
    };

    typedef yidl::runtime::auto_Object<Request> auto_Request;


    class Response : public Event
    {
    public:
      // yidl::runtime::Object
      Response& incRef() { return Object::incRef( *this ); }

    protected:
      Response() { }
      virtual ~Response() { }
    };

    typedef yidl::runtime::auto_Object<Response> auto_Response;


    class ExceptionResponse
      : public Response,
        public YIELD::platform::Exception
    {
    public:
      ExceptionResponse() { }

      ExceptionResponse( uint32_t error_code )
        : Exception( error_code )
      { }

      ExceptionResponse( const char* error_message )
        : Exception( error_message )
      { }

      ExceptionResponse( uint32_t error_code, const char* error_message )
        : Exception( error_code, error_message )
      { }

      ExceptionResponse( const Exception& other )
        : Exception( other )
      { }

      ExceptionResponse( const ExceptionResponse& other )
        : Exception( other )
      { }

      virtual ~ExceptionResponse() throw()
      { }

      virtual ExceptionResponse* clone() const
      {
        return new ExceptionResponse( *this );
      }

      virtual void throwStackClone() const
      {
        throw ExceptionResponse( *this );
      }

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( ExceptionResponse, 102 );
    };

    typedef yidl::runtime::auto_Object<ExceptionResponse>
      auto_ExceptionResponse;


    template <class ResponseType>
    class ResponseQueue
      : public EventTarget,
        private YIELD::platform::SynchronizedSTLQueue<Event*>
    {
    public:
      ResponseType& dequeue()
      {
        Event* dequeued_ev
          = YIELD::platform::SynchronizedSTLQueue<Event*>::dequeue();

        switch ( dequeued_ev->get_type_id() )
        {
          case YIDL_RUNTIME_OBJECT_TYPE_ID( ResponseType ):
          {
            return static_cast<ResponseType&>( *dequeued_ev );
          }
          break;

          case YIDL_RUNTIME_OBJECT_TYPE_ID( ExceptionResponse ):
          {
            try
            {
              static_cast<ExceptionResponse*>( dequeued_ev )
                ->throwStackClone();

              // Eliminate compiler warnings about control paths
              return static_cast<ResponseType&>( *dequeued_ev );
            }
            catch ( ExceptionResponse& )
            {
              Event::decRef( *dequeued_ev );
              throw;
            }
          }
          break;

          default:
          {
            throw YIELD::platform::Exception
                  (
                    "ResponseQueue::dequeue: received unexpected, " \
                    "non-exception event type"
                  );
          }
          break;
        }
      }

      void enqueue( Event& ev )
      {
        YIELD::platform::SynchronizedSTLQueue<Event*>::enqueue( &ev );
      }

      ResponseType& timed_dequeue( const YIELD::platform::Time& timeout )
      {
        Event* dequeued_ev
          = YIELD::platform::
              SynchronizedSTLQueue<Event*>::
                timed_dequeue( timeout );

        if ( dequeued_ev != NULL )
        {
          switch ( dequeued_ev->get_type_id() )
          {
            case YIDL_RUNTIME_OBJECT_TYPE_ID( ResponseType ):
            {
              return static_cast<ResponseType&>( *dequeued_ev );
            }
            break;

            case YIDL_RUNTIME_OBJECT_TYPE_ID( ExceptionResponse ):
            {
              try
              {
                static_cast<ExceptionResponse*>( dequeued_ev )
                  ->throwStackClone();
              }
              catch ( ExceptionResponse& )
              {
                Event::decRef( *dequeued_ev );
                throw;
              }

              throw YIELD::platform::Exception( "should never reach this point" );
            }
            break;

            default:
            {
              throw YIELD::platform::Exception
                   (
                     "ResponseQueue::dequeue: received unexpected, " \
                     " non-exception event type"
                   );
            }
            break;
          }
        }
        else
          throw YIELD::platform::Exception( "ResponseQueue::dequeue: timed out" );
      }

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( ResponseQueue<ResponseType>, 0 );

      // EventTarget
      void send( Event& ev )
      {
        enqueue( ev );
      }
    };

    template <class ResponseType>
    class auto_ResponseQueue
      : public yidl::runtime::auto_Object< ResponseQueue<ResponseType> >
    {
    public:
      auto_ResponseQueue( ResponseQueue<ResponseType>* response_queue )
        : yidl::runtime::auto_Object< ResponseQueue<ResponseType> >
         (
           response_queue
         )
      { }
    };



    class Stage : public EventTarget
    {
    public:
      class StartupEvent : public Event
      {
      public:
        StartupEvent( yidl::runtime::auto_Object<Stage> stage )
          : stage( stage )
        { }

        yidl::runtime::auto_Object<Stage> get_stage() { return stage; }

        // yidl::runtime::Object
        YIDL_RUNTIME_OBJECT_PROTOTYPES( Stage::StartupEvent, 104 );

      private:
        yidl::runtime::auto_Object<Stage> stage;
      };


      class ShutdownEvent : public Event
      {
      public:
        // yidl::runtime::Object
        YIDL_RUNTIME_OBJECT_PROTOTYPES( Stage::ShutdownEvent, 105 );
      };


      double get_arrival_rate_s() const { return arrival_rate_s; }
      double get_rho() const { return rho; }
      double get_service_rate_s() const { return service_rate_s; }
      uint8_t get_stage_id() const { return id; }
      const char* get_stage_name() const { return name; }
      virtual auto_EventHandler get_event_handler() = 0;
      virtual bool visit() = 0;
      virtual bool visit( const YIELD::platform::Time& timeout ) = 0;
      virtual void visit( Event& event ) = 0;

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( Stage, 103 );

    protected:
      Stage( const char* name );
      virtual ~Stage();

      YIELD::platform::Sampler<uint64_t, 1024, YIELD::platform::Mutex>
        event_processing_time_sampler;
      uint32_t event_queue_length, event_queue_arrival_count;
  #ifdef YIELD_PLATFORM_HAVE_PERFORMANCE_COUNTERS
      YIELD::platform::auto_PerformanceCounterSet performance_counters;
      uint64_t performance_counter_totals[2];
  #endif

    private:
      class StatisticsTimer;

      const char* name;
      uint8_t id;
      double arrival_rate_s, rho, service_rate_s;

      friend class StageGroup;
      void set_stage_id( uint8_t stage_id ) { this->id = stage_id; }
    };

    typedef yidl::runtime::auto_Object<Stage> auto_Stage;


    template <class EventHandlerType, class EventQueueType, class LockType>
    class StageImpl : public Stage
    {
    public:
      StageImpl
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler,
        yidl::runtime::auto_Object<EventQueueType> event_queue
      )
        : Stage( event_handler->get_type_name() ),
          event_handler( event_handler ),
          event_queue( event_queue )
      { }

      // EventTarget
      void send( Event& event )
      {
      /*
        Stage* running_stage = static_cast<Stage*>
          ( Thread::getTLS( running_stage_tls_key ) );
        if ( running_stage != NULL )
        {
          running_stage->send_counters_lock.acquire();
          std::map<const char*, uint64_t>::iterator send_counter_i
            = running_stage->send_counters.find( this->get_stage_name() );
          if ( send_counter_i != running_stage->send_counters.end() )
            send_counter_i->second++;
          else
            running_stage->send_counters.insert
              ( std::make_pair( this->get_stage_name(), 1 ) );
          running_stage->send_counters_lock.release();
        }
        */

        event.set_next_stage( this );

        ++event_queue_length;
        ++event_queue_arrival_count;

        if ( event_queue->enqueue( event ) )
          return;
        else
        {
          std::cerr << get_stage_name() << ": event queue full, stopping.";
          DebugBreak();
        }
      }

      // Stage
      const char* get_stage_name() const
      {
          return event_handler->get_type_name();
      }

      auto_EventHandler get_event_handler()
      {
        return event_handler->incRef();
      }

      bool visit()
      {
        lock.acquire();

        Event* event = event_queue->dequeue();
        if ( event != NULL )
        {
          --event_queue_length;
          callEventHandler( *event );

          for ( ;; )
          {
            event = event_queue->try_dequeue();
            if ( event != NULL )
            {
              --event_queue_length;
              callEventHandler( *event );
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

      bool visit( const YIELD::platform::Time& timeout )
      {
        if ( lock.try_acquire() )
        {
          Event* event = event_queue->timed_dequeue( timeout );
          if ( event != NULL )
          {
            --event_queue_length;
            callEventHandler( *event );

            for ( ;; )
            {
              event = event_queue->try_dequeue();
              if ( event != NULL )
              {
                --event_queue_length;
                callEventHandler( *event );
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
        callEventHandler( event );
        lock.release();
      }

    private:
      yidl::runtime::auto_Object<EventHandlerType> event_handler;
      yidl::runtime::auto_Object<EventQueueType> event_queue;

      LockType lock;

      void callEventHandler( Event& event )
      {
        YIELD::platform::Time start_time;

  #ifdef YIELD_PLATFORM_HAVE_PERFORMANCE_COUNTERS
        performance_counters->startCounting();
  #endif

        event_handler->handleEvent( event );

  #ifdef YIELD_PLATFORM_HAVE_PERFORMANCE_COUNTERS
        uint64_t performance_counter_counts[2];
        performance_counters->stopCounting( performance_counter_counts );
        performance_counter_totals[0] += performance_counter_counts[0];
        performance_counter_totals[1] += performance_counter_counts[1];
  #endif

        YIELD::platform::Time event_processing_time;
        event_processing_time -= start_time;
        if ( event_processing_time < 10.0 )
        {
          event_processing_time_sampler.
            set_next_sample( event_processing_time );
        }
      }
    };


    class StageGroup : public yidl::runtime::Object
    {
    public:
      template <class EventHandlerType>
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler
      )
      {
        return createStage
        (
          static_cast<EventHandler*>( event_handler.release() )
        );
      }

      template <class EventHandlerType>
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler,
        int16_t thread_count
      )
      {
        return createStage
        (
          static_cast<EventHandler*>
          (
            event_handler.release()
          ),
          thread_count
        );
      }

      virtual auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandler> event_handler,
        int16_t thread_count = 1
      ) = 0;

      Stage** get_stages() { return &stages[0]; }

      // yidl::runtime::Object
      StageGroup& incRef() { return Object::incRef( *this ); }

    protected:
      StageGroup();
      virtual ~StageGroup();

      void addStage( auto_Stage stage );

    private:
      Stage* stages[YIELD_STAGES_PER_GROUP_MAX];
    };

    typedef yidl::runtime::auto_Object<StageGroup> auto_StageGroup;


    template <class StageGroupType> // CRTP
    class StageGroupImpl : public StageGroup
    {
    public:
      template <class EventHandlerType>
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler
      )
      {
        return static_cast<StageGroupType*>( this )
          ->createStage<EventHandlerType, EventQueue>( event_handler, 1 );
      }

      template <class EventHandlerType>
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler,
        int16_t thread_count
      )
      {
        return static_cast<StageGroupType*>( this )
          ->createStage<EventHandlerType>( event_handler, thread_count );
      }

      // StageGroup
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandler> event_handler,
        int16_t thread_count = 1
      )
      {
        return createStage<EventHandler>( event_handler, thread_count );
      }

    protected:
      StageGroupImpl() { }
      virtual ~StageGroupImpl() { }
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
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler,
        int16_t
      )
      {
        auto_Stage stage;
        if ( event_handler->isThreadSafe() )
        {
          stage
            = new StageImpl
                  <
                    EventHandlerType,
                    STLEventQueue,
                    YIELD::platform::NOPLock
                  >( event_handler, event_queue );
        }
        else
        {
          stage
            = new StageImpl
                  <
                    EventHandlerType,
                    STLEventQueue,
                    YIELD::platform::Mutex
                  >( event_handler, event_queue );
        }

        event_handler->handleEvent( *( new Stage::StartupEvent( stage ) ) );

        this->addStage( stage );

        return stage;
      }

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( ColorStageGroup, 107 );

    private:
      auto_STLEventQueue event_queue;

      class Thread;
      std::vector<Thread*> threads;
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

      template <class EventHandlerType>
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler,
        int16_t
      )
      {
        auto_Stage stage;
        if ( use_thread_local_event_queues )
        {
          if ( event_handler->isThreadSafe() )
          {
            stage
              = new StageImpl
                    <
                      EventHandlerType,
                      ThreadLocalEventQueue,
                      YIELD::platform::NOPLock
                    >( event_handler, new ThreadLocalEventQueue );
          }
          else
          {
            stage
              = new StageImpl
                    <
                      EventHandlerType,
                      ThreadLocalEventQueue,
                      YIELD::platform::Mutex
                    >( event_handler, new ThreadLocalEventQueue );
          }
        }
        else
        {
          if ( event_handler->isThreadSafe() )
          {
            stage
              = new StageImpl
                    <
                      EventHandlerType,
                      STLEventQueue,
                      YIELD::platform::NOPLock
                    >( event_handler, new STLEventQueue );
          }
          else
          {
            stage
              = new StageImpl
                    <
                      EventHandlerType,
                      STLEventQueue,
                      YIELD::platform::Mutex
                    >( event_handler, new STLEventQueue );
          }
        }

        event_handler->handleEvent( *( new Stage::StartupEvent( stage ) ) );

        this->addStage( stage );

        return stage;
      }

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( PollingStageGroup<VisitPolicyType>, 0 );

    private:
      ~PollingStageGroup();

      bool use_thread_local_event_queues;

      class Thread;
      std::vector<Thread*> threads;
    };


    class DBRVisitPolicy : public VisitPolicy
    {
    public:
      DBRVisitPolicy( Stage** stages )
        : VisitPolicy( stages )
      {
        next_stage_i = YIELD_STAGES_PER_GROUP_MAX;
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
        else if ( next_stage_i < YIELD_STAGES_PER_GROUP_MAX )
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
            &sorted_stages[YIELD_STAGES_PER_GROUP_MAX-1],
            compare_stages()
          );

          next_stage_i = 0;

          return sorted_stages[0];
        }
      }

    private:
      uint8_t next_stage_i;
      Stage* sorted_stages[YIELD_STAGES_PER_GROUP_MAX];

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
      double last_rhos[YIELD_STAGES_PER_GROUP_MAX];

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
          next_stage_i = ( next_stage_i + 1 ) % YIELD_STAGES_PER_GROUP_MAX;

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
          if ( next_stage_i < YIELD_STAGES_PER_GROUP_MAX - 1 )
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
      auto_Stage createStage
      (
        yidl::runtime::auto_Object<EventHandlerType> event_handler,
        int16_t thread_count
      )
      {
        if ( thread_count <= 0 )
          thread_count
          = YIELD::platform::Machine::getOnlinePhysicalProcessorCount();

        auto_Stage stage;
        if ( event_handler->isThreadSafe() )
        {
          stage
            = new StageImpl
                  <
                    EventHandlerType,
                    STLEventQueue,
                    YIELD::platform::NOPLock
                  >( event_handler, new STLEventQueue );
        }
        else
        {
          stage
            = new StageImpl
                  <
                    EventHandlerType,
                    STLEventQueue,
                    YIELD::platform::Mutex
                  >( event_handler, new STLEventQueue );
        }

        event_handler->handleEvent( *( new Stage::StartupEvent( stage ) ) );

        this->addStage( stage );

        startThreads( stage, thread_count );

        return stage;
      }

      // yidl::runtime::Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( SEDAStageGroup, 106 );

    protected:
      virtual ~SEDAStageGroup();

    private:
      class Thread;
      std::vector<Thread*> threads;
      void startThreads( auto_Stage stage, int16_t thread_count );
    };
  };
};


#endif
