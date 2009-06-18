// Copyright 2003-2008 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef _YIELD_ARCH_H
#define _YIELD_ARCH_H

#include "yield/platform.h"


#define YIELD_STAGES_PER_GROUP_MAX 32

#ifdef __sun
#define YIELD_RECORD_PERFCTRS 1
#include <libcpc.h>
#endif


namespace YIELD
{
  class ExceptionResponse;
  class Request;
  class Response;


  class Event : public Object
  {
  public:
    // Object
    Event& incRef() { return Object::incRef( *this ); }

  protected:
    Event() : enqueued_time_ns( 0 ) { }
    virtual ~Event() { }

  private:
    template <class, class, class> friend class StageImpl;
    uint64_t enqueued_time_ns;
  };


  class EventTarget : public Object
  {
  public:
    virtual bool send( Event& ) = 0;

    // Object
    EventTarget& incRef() { return Object::incRef( *this ); }

  protected:
    EventTarget() { }
    virtual ~EventTarget() { }
  };


  class EventHandler : public EventTarget
  {
  public:
    virtual bool isThreadSafe() const { return false; }

    virtual void handleEvent( Event& ) = 0;
    virtual void handleUnknownEvent( Event& );

    // Object
    EventHandler& incRef() { return Object::incRef( *this ); }

    // EventTarget
    bool send( Event& );

  protected:
    EventHandler() { }
    virtual ~EventHandler() { }

  private:
    Mutex handleEvent_lock;
  };


  class EventQueue : public EventTarget
  {
  public:
    virtual Event* dequeue() = 0; // Blocking
    virtual Event* dequeue( uint64_t timeout_ns ) = 0;

    template <class ExpectedEventType>
    ExpectedEventType& dequeue_typed()
    {
      Event* dequeued_ev = dequeue();
      if ( dequeued_ev )
        return _checkDequeuedEventAgainstExpectedEventType<ExpectedEventType>( *dequeued_ev );
      else
        throw Exception( "EventQueue::dequeue_typed: blocking dequeue was broken" );
    }

    template <class ExpectedEventType>
    ExpectedEventType& dequeue_typed( uint64_t timeout_ns )
    {
      Event* dequeued_ev = dequeue( timeout_ns );
      if ( dequeued_ev )
        return _checkDequeuedEventAgainstExpectedEventType<ExpectedEventType>( *dequeued_ev );
      else
        throw Exception( "EventQueue::dequeue_typed: timed out" );
    }

    virtual bool enqueue( Event& ev ) = 0;
    virtual Event* try_dequeue() { return dequeue( 0 ); }

    // EventTarget
    bool send( Event& ev )
    {
      return enqueue( ev );
    }

  protected:
    EventQueue() { }
    virtual ~EventQueue() { }

  private:
    template <class ExpectedEventType>
    ExpectedEventType& _checkDequeuedEventAgainstExpectedEventType( Event& dequeued_ev );
  };


  template <class InternalQueueType = NonBlockingFiniteQueue<Event*, 2048> >
  class OneSignalEventQueue : public EventQueue, private InternalQueueType
  {
  public:
    ~OneSignalEventQueue() { }

    // Object
    YIELD_OBJECT_PROTOTYPES( OneSignalEventQueue, 101 );

    // EventQueue
    Event* dequeue()
    {
      Event* result = InternalQueueType::try_dequeue();

      if ( result != 0 ) // Hot case
        return result; // Don't dec the semaphore, just let a cold acquire succeed on an empty queue
      else
      {
        do
        {
          empty.acquire();
          result = InternalQueueType::try_dequeue();
        }
        while ( result == 0 );

        return result;
      }
    }

    Event* dequeue( uint64_t timeout_ns )
    {
      Event* result = InternalQueueType::try_dequeue();

      if ( result != 0 )
        return result;
      else
      {
        for ( ;; )
        {

          uint64_t before_acquire_time_ns = Time::getCurrentUnixTimeNS();

          empty.timed_acquire( timeout_ns );

          if ( ( result = InternalQueueType::try_dequeue() ) != 0 )
            break;

          uint64_t waited_ns = Time::getCurrentUnixTimeNS() - before_acquire_time_ns;
          if ( waited_ns >= timeout_ns )
            break;
          else
            timeout_ns -= static_cast<uint64_t>( waited_ns );
        }

        return result;
      }
    }

    bool enqueue( Event& ev )
    {
      if ( InternalQueueType::enqueue( &ev ) )
      {
        empty.release();
        return true;
      }
      else
        return false;
    }

    inline Event* try_dequeue()
    {
      return InternalQueueType::try_dequeue();
    }

  protected:
    CountingSemaphore empty;
  };


  class Interface : public EventHandler
  {
  public:
    virtual Request* checkRequest( Object& request ) = 0; // Casts an Object to a Request if the request belongs to the interface
    virtual Response* checkResponse( Object& response ) = 0; // Casts an Object to a Response if the request belongs to the interface
    virtual auto_Object<Request> createRequest( uint32_t tag ) = 0;
    virtual auto_Object<Response> createResponse( uint32_t tag ) = 0;    
    virtual auto_Object<ExceptionResponse> createExceptionResponse( uint32_t tag ) = 0;
  };


  class Response : public Event
  {
  public:
    // Object
    Response& incRef() { return Object::incRef( *this ); }

  protected:
    Response() { }
    virtual ~Response() { }
  };


  class Request : public Event
  {
  public:
    virtual bool respond( Response& response ) 
    { 
      if ( response_target != NULL )
        return response_target->send( response );
      else
      {
        Object::decRef( response );
        DebugBreak();
        return false;
      }
    }

    // Object
    Request& incRef() { return Object::incRef( *this ); }

    auto_Object<EventTarget> get_response_target() const { return response_target; }
    void set_response_target( auto_Object<EventTarget> response_target ) { this->response_target = response_target; }

  protected:
    Request() { }
    virtual ~Request() { }

  private:
    auto_Object<EventTarget> response_target;
  };


  class ExceptionResponse : public Response, public Exception
  {
  public:
    ExceptionResponse() { }
    ExceptionResponse( uint32_t error_code ) : Exception( error_code ) { }
    ExceptionResponse( const char* what ) : Exception( what ) { }
    ExceptionResponse( const Exception& other ) : Exception( other ) { }
    ExceptionResponse( const ExceptionResponse& other ) : Exception( other ) { }
    virtual ~ExceptionResponse() throw() { }

    virtual ExceptionResponse* clone() const { return new ExceptionResponse( what() ); }
    virtual void throwStackClone() const { throw ExceptionResponse( what() ); }

    // Object
    YIELD_OBJECT_PROTOTYPES( ExceptionResponse, 102 );
  };


  template <class ExpectedEventType>
  ExpectedEventType& EventQueue::_checkDequeuedEventAgainstExpectedEventType( Event& dequeued_ev )
  {
    switch ( dequeued_ev.get_tag() )
    {
      case YIELD_OBJECT_TAG( ExpectedEventType ): return static_cast<ExpectedEventType&>( dequeued_ev );

      case YIELD_OBJECT_TAG( ExceptionResponse ):
      {
        try
        {
          static_cast<ExceptionResponse&>( dequeued_ev ).throwStackClone();
        }
        catch ( ExceptionResponse& )
        {
          Object::decRef( dequeued_ev );
          throw;
        }

        throw Exception( "should never reach this point" );
      }

     default: throw Exception( "EventQueue::deqeue_typed: received unexpected, non-exception event type" );
    }        
  }


  class Stage : public EventTarget
  {
  public:
    virtual const char* get_stage_name() const = 0;
    virtual auto_Object<EventHandler> get_event_handler() = 0;
    virtual auto_Object<EventQueue> get_event_queue() = 0;

    virtual double get_rho() const { return 0; }

    virtual bool visit() { return false; }; // Blocking visit, for SEDA
    virtual bool visit( uint64_t ) { return false; }; // Timed visit, for CohortS

    // Object
    YIELD_OBJECT_PROTOTYPES( Stage, 103 );

  protected:
    Stage()
    {
#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
      last_recorded_cpc_events_time_ns = 0;
      pic0_total = pic1_total = 0;
#endif
#endif
    }

    virtual ~Stage() { }

  private:
#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
    friend class StageGroupThread;
    uint64_t pic0_total, pic1_total;
    uint64_t last_recorded_cpc_events_time_ns;
#endif
#endif
  };


  // A separate templated version for subclasses to inherit from (MG1Stage, CohortStage, etc.)
  // This helps eliminate some virtual function calls by having the specific EventHandler type instead of a generic EventHandler pointer
  template <class EventHandlerType, class EventQueueType, class LockType>
  class StageImpl : public Stage//, private StatsEventSource<LockType>
  {
  public:
    StageImpl( auto_Object<EventHandlerType> event_handler, auto_Object<EventQueueType> event_queue, auto_Object<EventTarget>, auto_Object<Log> log )
      : //StatsEventSource<LockType>( 2000, stage_stats_event_target ),
        event_handler( event_handler ), event_queue( event_queue ), log( log )
    {      
      event_queue_length = event_queue_arrival_count = 1; // send() would normally inc these, but we can't use send() because it's a virtual function; instead we enqueue directly and inc the lengths ourselves
      event_queue_full_count = 0;

      event_queue_length = event_queue_arrival_count = 0;
      rho = elapsed_ms = 0;
    }

    double get_rho() const { return rho; }

    // EventTarget
    bool send( Event& ev )
    {
      if ( log != NULL && log->get_level() >= Log::LOG_DEBUG )
      {
        std::ostringstream log_str;
        log_str << "yield::StageImpl: thread #" << Thread::getCurrentThreadId() << " sending " << ev.get_type_name() << " to the " << get_stage_name() << " stage.";
        log->getStream( Log::LOG_DEBUG ) << log_str.str();
      }

      ++event_queue_length;
      ++event_queue_arrival_count;

      if ( event_queue->enqueue( ev ) )
        return true;
      else
      {
        if ( event_queue_full_count++ < 10 )
          std::cerr << "yield::StageImpl: " << Time() << ": queue full #" << static_cast<unsigned short>( event_queue_full_count ) << " at " << event_handler->get_type_name() << " stage." << std::endl;

        /*
        StageGroupThread* running_stage_group_thread = stage_group.get_running_stage_group_thread();

        for ( unsigned char lock_acquire_tries = 0; lock_acquire_tries < 15; lock_acquire_tries++ )
        {
          // Acquire the lock on the receiving stage
          if ( lock.timed_acquire( static_cast<uint64_t>( lock_acquire_tries * NS_IN_MS ) ) )
          {
            while ( running_stage_group_thread->shouldRun() )
            {
              // Keep trying to enqueue until it succeeds
              // Process an event from the head of the queue
              Event* head_ev = event_queue->try_dequeue();
              if ( head_ev )
              {
                --event_queue_length;
                _callEventHandler( *head_ev );
              }

              // Try to enqueue the send() event
              if ( event_queue->enqueue( ev ) )
              {
                lock.release();
                return true;
              }
            }
          }
        }
        */

        return false;
      }
    }

    // Stage
    const char* get_stage_name() const { return event_handler->get_type_name(); }
    auto_Object<EventHandler> get_event_handler() { return event_handler->incRef(); }
    auto_Object<EventQueue> get_event_queue() { return static_cast<EventQueue&>( event_queue->incRef() ); }

    bool visit()
    {
      if ( lock.try_acquire() )
      {
        Event* ev = event_queue->dequeue();
        if ( ev != NULL )
        {
          --event_queue_length;
          _callEventHandler( *ev );
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

    bool visit( uint64_t timeout_ns )
    {
      if ( lock.try_acquire() )
      {
        Event* ev = event_queue->dequeue( timeout_ns );
        if ( ev != NULL )
        {
          --event_queue_length;
          _callEventHandler( *ev );
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

  private:
    auto_Object<EventHandlerType> event_handler;
    auto_Object<EventQueueType> event_queue;
    auto_Object<Log> log;

    unsigned char event_queue_full_count;
    LockType lock;


    void _callEventHandler( Event& ev )
    {
      uint64_t event_queueing_time_ns = Time::getCurrentUnixTimeNS() - ev.enqueued_time_ns;
      if ( event_queueing_time_ns < 2 * NS_IN_S )
        event_queueing_time_sampler.setNextSample( static_cast<double>( event_queueing_time_ns * NS_IN_MS ) );

      if ( log != NULL && log->get_level() >= Log::LOG_DEBUG )
      {
        std::ostringstream log_str;
        log_str << "yield::StageImpl: thread #" << Thread::getCurrentThreadId() << " processing " << ev.get_type_name() << " at the " << get_stage_name() << " stage.";
        log->getStream( Log::LOG_DEBUG ) << log_str.str();
      }

      double start_time_ms = Time::getCurrentUnixTimeMS();

      event_handler->handleEvent( ev );

      double event_processing_time_ms = Time::getCurrentUnixTimeMS() - start_time_ms;
      if ( event_processing_time_ms < 10 * MS_IN_S )
        event_processing_time_sampler.setNextSample( event_processing_time_ms );

      // StatsEventSource<LockType>::checkTimer();
    }


    Sampler<double, 2000, Mutex> event_queueing_time_sampler, event_processing_time_sampler;
    Sampler<uint32_t, 2000, Mutex> queue_length_sampler;
    Sampler<double, 2000, Mutex> arrival_rate_sampler;
    uint32_t event_queue_length, event_queue_arrival_count;
    double rho, elapsed_ms;

    /*
    // StatsEventSource
    StatsEvent* createStatsEvent( double elapsed_ms )
    {
      elapsed_ms += elapsed_ms;

      uint32_t event_queue_length = this->event_queue_length;
      queue_length_sampler.setNextSample( event_queue_length );
      double arrival_rate = static_cast<double>( event_queue_arrival_count ) / elapsed_ms;
      event_queue_arrival_count = 0;
      arrival_rate_sampler.setNextSample( arrival_rate );

      if ( elapsed_ms < 1000 )
        return NULL;
      else
      {
        StageStatsEvent* stage_stats_ev = new StageStatsEvent( get_event_handler().get_type_name() );

        stage_stats_ev->queue_length_samples_count = queue_length_sampler.getSamplesCount();
        stage_stats_ev->queue_length_statistics = queue_length_sampler.getStatistics();
        stage_stats_ev->event_queueing_time_statistics = event_queueing_time_sampler.getStatistics();
        stage_stats_ev->event_processing_time_statistics = event_processing_time_sampler.getStatistics();

        stage_stats_ev->arrival_rate_s = arrival_rate_sampler.getStatistics().ninetieth_percentile * 1000.0;
        stage_stats_ev->service_rate_s = 1000.0 / static_cast<uint64_t>( stage_stats_ev->event_processing_time_statistics.ninetieth_percentile );
        stage_stats_ev->rho = rho = ( stage_stats_ev->service_rate_s > 0 ) ? stage_stats_ev->arrival_rate_s / stage_stats_ev->service_rate_s : 0;

        return stage_stats_ev;
      }
    }
    */
  };


  class StageGroupThread;

  class StageGroup : public Object
  {
  public:
    virtual auto_Object<Stage> createStage( auto_Object<EventHandler> event_handler, 
                                            int16_t thread_count = 1, 
                                            auto_Object<EventQueue> event_queue = NULL, 
                                            auto_Object<EventTarget> stage_stats_event_target = NULL, 
                                            auto_Object<Log> log = NULL ) = 0;

    auto_Object<ProcessorSet> get_limit_physical_processor_set() const { return limit_physical_processor_set; }
    auto_Object<ProcessorSet> get_limit_logical_processor_set() const { return limit_logical_processor_set; }
    const std::string& get_name() const { return name; }
    unsigned long get_running_stage_group_thread_tls_key() const { return running_stage_group_thread_tls_key; }
    inline StageGroupThread* get_running_stage_group_thread() const { return static_cast<StageGroupThread*>( Thread::getTLS( running_stage_group_thread_tls_key ) ); }

    // Object
    StageGroup& incRef() { return Object::incRef( *this ); }

  protected:
    StageGroup( const std::string& name, auto_Object<ProcessorSet> limit_physical_processor_set, auto_Object<EventTarget> stage_stats_event_target );
    virtual ~StageGroup() { }

    auto_Object<EventTarget> get_stage_stats_event_target() const { return stage_stats_event_target; }

  private:
    std::string name;
    auto_Object<ProcessorSet> limit_physical_processor_set, limit_logical_processor_set;
    auto_Object<EventTarget> stage_stats_event_target;

    unsigned long running_stage_group_thread_tls_key;
  };

    
  template <class StageGroupType> // CRTP
  class StageGroupImpl : public StageGroup 
  {
  public:
    // Templated createStage's that pass the real EventHandler and EventQueue types to StageImpl to bypass the interfaces
    template <class EventHandlerType>
    auto_Object<Stage> createStage( auto_Object<EventHandlerType> event_handler, int16_t thread_count, auto_Object<EventQueue> event_queue, auto_Object<EventTarget> stage_stats_event_target, auto_Object<Log> log )
    {
      return static_cast<StageGroupType*>( this )->createStage< EventHandlerType, OneSignalEventQueue<> >( event_handler, thread_count, new OneSignalEventQueue<>, stage_stats_event_target, log );
    }

    template <class EventHandlerType>
    auto_Object<Stage> createStage( auto_Object<EventHandlerType> event_handler, auto_Object<Log> log )
    {
      return static_cast<StageGroupType*>( this )->createStage< EventHandlerType, OneSignalEventQueue<> >( event_handler, 1, new OneSignalEventQueue<>, NULL, log );
    }

    // StageGroup
    auto_Object<Stage> createStage( auto_Object<EventHandler> event_handler, int16_t thread_count = 1, auto_Object<EventQueue> event_queue = NULL, auto_Object<EventTarget> stage_stats_event_target = NULL, auto_Object<Log> log = NULL )
    {
      if ( event_queue == NULL )
        return createStage<EventHandler>( event_handler, thread_count, event_queue, stage_stats_event_target, log );
      else
        return static_cast<StageGroupType*>( this )->createStage<EventHandler, EventQueue>( event_handler, thread_count, event_queue, stage_stats_event_target, log );
    }

  protected:
    StageGroupImpl( const std::string& name, auto_Object<ProcessorSet> limit_physical_processor_set, auto_Object<EventTarget> stage_stats_event_target, auto_Object<Log> log )
      : StageGroup( name, limit_physical_processor_set, stage_stats_event_target ), log( log )
    {
      memset( stages, 0, sizeof( stages ) );
    }

    virtual ~StageGroupImpl()
    {
      for ( uint8_t stage_i = 0; stage_i < YIELD_STAGES_PER_GROUP_MAX; stage_i++ )
        Object::decRef( stages[stage_i] );
    }

    auto_Object<Log> log;

    Stage* stages[YIELD_STAGES_PER_GROUP_MAX];

    void addStage( auto_Object<Stage> stage )
    {
      unsigned char stage_i;
      for ( stage_i = 0; stage_i < YIELD_STAGES_PER_GROUP_MAX; stage_i++ )
      {
        if ( stages[stage_i] == NULL )
        {
          stages[stage_i] = stage.release();
          return;
        }
      }

      DebugBreak();
    }

    auto_Object<Log> get_log() const { return log; }
  };


  class StageGroupThread : public Thread
  {
  public:
    virtual ~StageGroupThread();

    virtual void stop() = 0;

    // Thread
    void run();
    virtual void start();

  protected:
    StageGroupThread( const std::string& stage_group_name, auto_Object<ProcessorSet> limit_logical_processor_set = NULL, auto_Object<Log> = NULL );


    bool is_running, should_run;


    virtual void _run() = 0;

    inline bool visitStage( Stage& stage )
    {
#ifdef YIELD_RECORD_PERFCTRS
      startPerformanceCounterSampling();
#endif

      bool success = stage.visit();

#ifdef YIELD_RECORD_PERFCTRS
      stopPerformanceCounterSampling( stage, success );
#endif

      return success;
    }

    inline bool visitStage( Stage& stage, uint64_t timeout_ns )
    {
#ifdef YIELD_RECORD_PERFCTRS
      startPerformanceCounterSampling();
#endif

      bool success = stage.visit( timeout_ns );

#ifdef YIELD_RECORD_PERFCTRS
      stopPerformanceCounterSampling( stage, success );
#endif

      return success;
    }

  private:
    std::string stage_group_name;
    auto_Object<ProcessorSet> limit_logical_processor_set;
    auto_Object<Log> log;

#ifdef YIELD_RECORD_PERFCTRS
#ifdef __sun
    cpc_t* cpc; cpc_set_t* cpc_set;
    int pic0_index, pic1_index;
    cpc_buf_t *before_cpc_buf, *after_cpc_buf, *diff_cpc_buf;

    inline void startPerformanceCounterSampling()
    {
      cpc_set_sample( cpc, cpc_set, before_cpc_buf );
    }

    inline void stopPerformanceCounterSampling( Stage& stage, bool visit_was_successful )
    {
      cpc_set_sample( cpc, cpc_set, after_cpc_buf );
      cpc_buf_sub( cpc, diff_cpc_buf, after_cpc_buf, before_cpc_buf );
      uint64_t pic0; cpc_buf_get( cpc, diff_cpc_buf, pic0_index, &pic0 );
      stage.pic0_total += pic0;
      uint64_t pic1; cpc_buf_get( cpc, diff_cpc_buf, pic1_index, &pic1 );
      stage.pic1_total += pic1;

      if ( visit_was_successful )
      {
        uint64_t current_time_ns = Time::getCurrentUnixTimeNS();
        if ( stage.last_recorded_cpc_events_time_ns - current_time_ns >= 5 * NS_IN_S )
        {
          stage.last_recorded_cpc_events_time_ns = current_time_ns;
          std::ostringstream cout_str;
          cout_str << stage.get_stage_name() << " performance counter totals: " << stage.pic0_total << "/" << stage.pic1_total << std::endl;
          std::cout << cout_str.str();
        }
      }
    }
#endif
#endif
  };


  class StageStartupEvent : public Event
  {
  public:
    StageStartupEvent( auto_Object<Stage> stage ) 
      : stage( stage )
    { }

    auto_Object<Stage> get_stage() { return stage; }

    // Object
    YIELD_OBJECT_PROTOTYPES( StageStartupEvent, 104 );

  private:
    auto_Object<Stage> stage;
  };


  class StageShutdownEvent : public Event
  {
  public:
    // Object
    YIELD_OBJECT_PROTOTYPES( StageShutdownEvent, 105 );
  };


  class SEDAStageGroupThread;


  class SEDAStageGroup : public StageGroupImpl<SEDAStageGroup>
  {
  public:
    SEDAStageGroup( const char* name, ProcessorSet* limit_physical_processor_set = NULL, auto_Object<EventTarget> stage_stats_event_target = NULL, auto_Object<Log> log = NULL )
        : StageGroupImpl<SEDAStageGroup>( name, limit_physical_processor_set, stage_stats_event_target, log )
    { }

    template <class EventHandlerType, class EventQueueType>
    auto_Object<Stage> createStage( auto_Object<EventHandlerType> event_handler, int16_t thread_count, auto_Object<EventQueueType> event_queue, auto_Object<EventTarget> stage_stats_event_target = NULL, auto_Object<Log> log = NULL )
    {
      if ( thread_count == -1 )
      {
        if ( get_limit_physical_processor_set() != NULL )
          thread_count = get_limit_physical_processor_set()->count();
        else
          thread_count = Machine::getOnlinePhysicalProcessorCount();
      }

      if ( stage_stats_event_target == NULL )
        stage_stats_event_target = this->get_stage_stats_event_target();

      if ( log == NULL )
        log = this->get_log();

      auto_Object<Stage> stage;
      if ( event_handler->isThreadSafe() )
        stage = new StageImpl<EventHandlerType, EventQueueType, NOPLock>( event_handler, event_queue, stage_stats_event_target, log );
      else
        stage = new StageImpl<EventHandlerType, EventQueueType, Mutex>( event_handler, event_queue, stage_stats_event_target, log );

      this->addStage( stage );
      startThreads( stage, thread_count );

      return stage;
    }

    // Object
    YIELD_OBJECT_PROTOTYPES( SEDAStageGroup, 106 );

  protected:
    virtual ~SEDAStageGroup();

  private:
    std::vector<SEDAStageGroupThread*> threads;
    void startThreads( auto_Object<Stage> stage, int16_t thread_count );
  };


  class TimerEventQueue : public EventQueue, private NonBlockingFiniteQueue<Event*, 2048>
  {
  public:
    class TimerEvent : public Event
    {
    public:
      TimerEvent( const Time& timeout, const Time& period, auto_Object<> context = NULL )
        : context( context ), 
          fire_time( Time() + timeout ), 
          timeout( timeout ), period( period )
      { }

      auto_Object<> get_context() const { return context; }
      const Time& get_fire_time() const { return fire_time; }
      const Time& get_period() const { return period; }
      const Time& get_timeout() const { return timeout; }

      // Object
      YIELD_OBJECT_PROTOTYPES( TimerEvent, 218 );    

    private:
      ~TimerEvent() { }

      auto_Object<> context;
      Time fire_time, timeout, period;
    };

    
    TimerEventQueue();
    virtual ~TimerEventQueue();

    uint64_t getNSUntilNextTimer();

    auto_Object<TimerEvent> timer_create( const Time& timeout, auto_Object<> context = NULL ) { return timer_create( timeout, Time( static_cast<uint64_t>( 0 ) ), context ); }
    auto_Object<TimerEvent> timer_create( const Time& timeout, const Time& period, auto_Object<> context = NULL );

    // EventQueue
    virtual bool enqueue( Event& );
    virtual Event* dequeue() { return dequeue( static_cast<uint64_t>( -1 ) ); }
    virtual Event* dequeue( uint64_t timeout_ns );
    virtual Event* try_dequeue();

  private:
    std::vector<TimerEvent*> timers;


    static bool compareTimerEvents( const TimerEvent*, const TimerEvent* );
  };
};

#endif
