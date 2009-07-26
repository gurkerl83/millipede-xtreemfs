// Copyright 2003-2009 Minor Gordon, with original implementations and ideas contributed by Felix Hupfeld.
// This source comes from the Yield project. It is licensed under the GPLv2 (see COPYING for terms and conditions).

#ifndef _YIELD_BASE_H_
#define _YIELD_BASE_H_

#define __STDC_LIMIT_MACROS
#ifdef _WIN32
#include "msstdint.h"
#else
#include <stdint.h>
#endif

#include <cstring>
#include <ostream>
#include <string>

#ifdef _WIN32
extern "C"
{
  __declspec( dllimport ) void __stdcall DebugBreak();
}
#else
#include <sys/uio.h> // For struct iovec
#endif

#include "atomic.h"

#ifdef __sun
#define YIELD yield_
#else
#define YIELD yield
#endif

// #define YIELD_DEBUG_REFERENCE_COUNTING 1

#define YIELD_OBJECT_PROTOTYPES( type_name, type_id ) \
    type_name & incRef() { return ::YIELD::Object::incRef( *this ); } \
    const static uint32_t __type_id = static_cast<uint32_t>( type_id ); \
    virtual uint32_t get_type_id() const { return __type_id; } \
    const char* get_type_name() const { return #type_name; }

#define YIELD_OBJECT_TYPE_ID( type ) type::__type_id


#ifdef _WIN32

struct iovec
{
  size_t iov_len;
  void* iov_base;
};

#else

inline void memcpy_s( void* dest, size_t dest_size, const void* src, size_t count )
{
  memcpy( dest, src, count );
}

#ifndef __linux__
inline size_t strnlen( const char* s, size_t maxlen )
{
  return strlen( s );
}
#endif

#endif


namespace YIELD
{
  class Marshaller;
  class Sequence;
  class Struct;
  class Unmarshaller;


#ifdef _WIN32
  static inline void DebugBreak()
  {
    ::DebugBreak();
  }
#else
  static inline void DebugBreak()
  {
    *((int*)0) = 0xabadcafe;
  }
#endif


  class Object
  {
  public:
    Object() : refcnt( 1 )
    { }

    static inline void decRef( Object& object )
    {
#ifdef YIELD_DEBUG_REFERENCE_COUNTING
      if ( atomic_dec( &object.refcnt ) < 0 )
        DebugBreak();
#else
      if ( atomic_dec( &object.refcnt ) == 0 )
        delete &object;
#endif
    }

    static inline void decRef( Object* object )
    {
      if ( object )
        Object::decRef( *object );
    }

    template <class ObjectType>
    static inline ObjectType& incRef( ObjectType& object )
    {
#ifdef YIELD_DEBUG_REFERENCE_COUNTING
      if ( object.refcnt <= 0 )
        DebugBreak();
#endif
      atomic_inc( &object.refcnt );
      return object;
    }

    template <class ObjectType>
    static inline ObjectType* incRef( ObjectType* object )
    {
      if ( object )
        incRef( *object );
      return object;
    }

    inline Object& incRef()
    {
      incRef( *this );
      return *this;
    }

    virtual uint32_t get_type_id() const = 0;
    virtual const char* get_type_name() const = 0;
    virtual void marshal( Marshaller& ) const { }
    virtual void unmarshal( Unmarshaller& ) { }

  protected:
    virtual ~Object()
    { }

  private:
    volatile int32_t refcnt;
  };


  template <class ObjectType = Object>
  class auto_Object // Like auto_ptr, but using Object::decRef instead of delete; an operator delete( void* ) on Object doesn't work, because the object is destructed before that call
  {
  public:
    auto_Object() : object( 0 ) { }
    auto_Object( ObjectType* object ) : object( object ) { }
    auto_Object( ObjectType& object ) : object( &object ) { }
    auto_Object( const auto_Object<ObjectType>& other ) { object = Object::incRef( other.object ); }
    ~auto_Object() { Object::decRef( object ); }

    inline ObjectType* get() const { return object; }
    auto_Object& operator=( const auto_Object<ObjectType>& other ) { Object::decRef( this->object ); object = Object::incRef( other.object ); return *this; }
    auto_Object& operator=( ObjectType* object ) { Object::decRef( this->object ); this->object = object; return *this; }
    inline bool operator==( const auto_Object<ObjectType>& other ) const { return object == other.object; }
    inline bool operator==( const ObjectType* other ) const { return object == other; }
    inline bool operator!=( const ObjectType* other ) const { return object != other; }
    // operator ObjectType*() const { return object; } // Creates sneaky bugs
    inline ObjectType* operator->() const { return get(); }
    inline ObjectType& operator*() const { return *get(); }
    inline ObjectType* release() { ObjectType* temp_object = object; object = 0; return temp_object; }
    inline void reset( ObjectType* object ) { Object::decRef( this->object ); this->object = object; }

  private:
    ObjectType* object;
  };


  class Buffer : public Object
  {
  public:
    virtual ~Buffer() { }

    virtual size_t capacity() const = 0;
    bool empty() const { return size() == 0; }    
    virtual size_t get( void* into_buffer, size_t into_buffer_len ) = 0;
    operator char*() const { return static_cast<char*>( static_cast<void*>( *this ) ); }
    operator unsigned char*() const { return static_cast<unsigned char*>( static_cast<void*>( *this ) ); }
    virtual operator void*() const = 0;
    bool operator==( const Buffer& other ) const;
    size_t put( const char* from_string ) { return put( from_string, strlen( from_string ) ); }
    size_t put( const std::string& from_string ) { return put( from_string.c_str(), from_string.size() ); }
    virtual size_t put( const void* from_buffer, size_t from_buffer_len ) = 0;
    virtual size_t size() const = 0;

    // Object
    YIELD_OBJECT_PROTOTYPES( Buffer, 1 );
  };

  typedef auto_Object<Buffer> auto_Buffer;


  class FixedBuffer : public Buffer
  {
  public:
    bool operator==( const FixedBuffer& other ) const;

    // Object
    YIELD_OBJECT_PROTOTYPES( FixedBuffer, 2 );

    // Buffer
    size_t get( void* into_buffer, size_t into_buffer_len );
    size_t capacity() const;
    operator iovec() const { return iov; }
    operator void*() const;
    virtual size_t put( const void* from_buffer, size_t from_buffer_len );
    size_t size() const;

  protected:
    FixedBuffer( size_t capacity );

    struct iovec iov;

  private:
    size_t _capacity;
    size_t _consumed; // Total number of bytes consumed by get()
  };


  class GatherBuffer : public Buffer
  {
  public:
    GatherBuffer( const struct iovec* iovecs, uint32_t iovecs_len );

    const struct iovec* get_iovecs() const { return iovecs; }
    uint32_t get_iovecs_len() const { return iovecs_len; }

    // Object
    YIELD_OBJECT_PROTOTYPES( GatherBuffer, 3 );

    // Buffer
    size_t capacity() const { return size(); }
    size_t get( void*, size_t ) { return 0; }
    size_t put( const void*, size_t ) { return 0; }
    operator void*() const;
    size_t size() const;

  private:
    const struct iovec* iovecs;
    uint32_t iovecs_len;
  };


  class HeapBuffer : public FixedBuffer
  {
  public:
    HeapBuffer( size_t capacity );
    virtual ~HeapBuffer();

    // Object
    YIELD_OBJECT_PROTOTYPES( HeapBuffer, 4 );
  };


  class Map : public Object
  {
  public:
    virtual size_t get_size() const = 0;
  };


  class Marshaller
  {
  public:
    virtual ~Marshaller() { }

    virtual void writeBoolean( const char* key, uint32_t tag, bool value ) = 0;
    virtual void writeBuffer( const char* key, uint32_t tag, auto_Buffer value ) = 0;
    virtual void writeFloat( const char* key, uint32_t tag, float value ) { writeDouble( key, tag, value ); }
    virtual void writeDouble( const char* key, uint32_t tag, double value ) = 0;
    virtual void writeInt8( const char* key, uint32_t tag, int8_t value ) { writeInt16( key, tag, value ); }
    virtual void writeInt16( const char* key, uint32_t tag, int16_t value ) { writeInt32( key, tag, value ); }
    virtual void writeInt32( const char* key, uint32_t tag, int32_t value ) { writeInt64( key, tag, value ); }
    virtual void writeInt64( const char* key, uint32_t tag, int64_t value ) = 0;
    virtual void writeMap( const char* key, uint32_t tag, const Map& value ) = 0;
    virtual void writeSequence( const char* key, uint32_t tag, const Sequence& value ) = 0;
    virtual void writeStruct( const char* key, uint32_t tag, const Struct& value ) = 0;
    virtual void writeString( const char* key, uint32_t tag, const std::string& value ) { writeString( key, tag, value.c_str(), value.size() ); }
    virtual void writeString( const char* key, uint32_t tag, const char* value ) { writeString( key, tag, value, strnlen( value, UINT16_MAX ) ); }
    virtual void writeString( const char* key, uint32_t tag, const char* value, size_t value_len ) = 0;
    virtual void writeUint8( const char* key, uint32_t tag, uint8_t value ) { writeInt8( key, tag, static_cast<int8_t>( value ) ); }
    virtual void writeUint16( const char* key, uint32_t tag, uint16_t value ) { writeInt16( key, tag, static_cast<int16_t>( value ) ); }
    virtual void writeUint32( const char* key, uint32_t tag, uint32_t value ) { writeInt32( key, tag, static_cast<int32_t>( value ) ); }
    virtual void writeUint64( const char* key, uint32_t tag, uint64_t value ) { writeInt64( key, tag, static_cast<int64_t>( value ) ); }
  };

#define YIELD_MARSHALLER_PROTOTYPES \
  virtual void writeBoolean( const char* key, uint32_t tag, bool value ); \
  virtual void writeBuffer( const char* key, uint32_t tag, auto_Buffer value ); \
  virtual void writeDouble( const char* key, uint32_t tag, double value ); \
  virtual void writeInt64( const char* key, uint32_t tag, int64_t value ); \
  virtual void writeMap( const char* key, uint32_t tag, const YIELD::Map& value ); \
  virtual void writeSequence( const char* key, uint32_t tag, const YIELD::Sequence& value ); \
  virtual void writeString( const char* key, uint32_t tag, const char* value, size_t value_len ); \
  virtual void writeStruct( const char* key, uint32_t tag, const YIELD::Struct& value );


  class PrettyPrinter : public Marshaller
  {
  public:
    PrettyPrinter( std::ostream& os )
      : os( os )
    { }

    PrettyPrinter& operator=( const PrettyPrinter& ) { return *this; }

    // Marshaller
    YIELD_MARSHALLER_PROTOTYPES;

  private:
    std::ostream& os;
  };


  class Sequence : public Object
  {
  public:
    virtual size_t get_size() const = 0;
  };


  template <size_t Capacity>
  class StackBuffer : public FixedBuffer
  {
  public:
    StackBuffer()
      : FixedBuffer( Capacity )
    {
      iov.iov_base = _stack_buffer;
    }

    StackBuffer( const void* from_buffer )
      : FixedBuffer( Capacity )
    {
      iov.iov_base = _stack_buffer;
      memcpy_s( _stack_buffer, Capacity, from_buffer, Capacity );
      iov.iov_len = Capacity;
    }

    // Object
    YIELD_OBJECT_PROTOTYPES( StackBuffer, 5 );

  private:
    uint8_t _stack_buffer[Capacity];
  };


  class StringBuffer : public Buffer
  {
  public:
    StringBuffer();
    StringBuffer( size_t capacity );
    StringBuffer( const std::string& );
    StringBuffer( const char* );
    StringBuffer( const char*, size_t );

    const char* c_str() const { return _string.c_str(); }
    operator std::string&() { return _string; }
    operator const std::string&() const { return _string; }
    operator void*() const { return const_cast<char*>( _string.c_str() ); }
    bool operator==( const StringBuffer& other ) const { return _string == other._string; }
    bool operator==( const char* other ) const { return _string == other; }

    // Object
    YIELD_OBJECT_PROTOTYPES( StringBuffer, 6 );

    // Buffer
    size_t capacity() const { return _string.capacity(); }
    size_t get( void* into_buffer, size_t into_buffer_len );
    size_t put( const void*, size_t );
    size_t size() const;

  private:
    std::string _string;

    size_t _consumed;
  };

  typedef auto_Object<StringBuffer> auto_StringBuffer;


  class StringLiteralBuffer : public FixedBuffer
  {
  public:
    StringLiteralBuffer( const char* string_literal );
    StringLiteralBuffer( const char* string_literal, size_t string_literal_len );
    StringLiteralBuffer( const void* string_literal, size_t string_literal_len );

    // Object
    YIELD_OBJECT_PROTOTYPES( StringLiteralBuffer, 7 );

    // Buffer
    size_t put( const void*, size_t ) { return 0; }
  };


  class Struct : public Object
  { };

  typedef auto_Object<Struct> auto_Struct;


  class Unmarshaller
  {
  public:
    virtual ~Unmarshaller() { }

    virtual bool readBoolean( const char* key, uint32_t tag ) = 0;
    virtual void readBuffer( const char* key, uint32_t tag, auto_Buffer value ) = 0;
    virtual double readDouble( const char* key, uint32_t tag ) = 0;
    virtual float readFloat( const char* key, uint32_t tag ) { return static_cast<float>( readDouble( key, tag ) ); }
    virtual int8_t readInt8( const char* key, uint32_t tag ) { return static_cast<int8_t>( readInt16( key, tag ) ); }
    virtual int16_t readInt16( const char* key, uint32_t tag ) { return static_cast<int16_t>( readInt32( key, tag ) ); }
    virtual int32_t readInt32( const char* key, uint32_t tag ) { return static_cast<int32_t>( readInt64( key, tag ) ); }
    virtual int64_t readInt64( const char* key, uint32_t tag ) = 0;
    virtual void readMap( const char* key, uint32_t tag, Map& value ) = 0;
    virtual void readSequence( const char* key, uint32_t tag, Sequence& value ) = 0;
    virtual void readString( const char* key, uint32_t tag, std::string& value ) = 0;
    virtual void readStruct( const char* key, uint32_t tag, Struct& value ) = 0;
    virtual uint8_t readUint8( const char* key, uint32_t tag ) { return static_cast<uint8_t>( readInt8( key, tag ) ); }
    virtual uint16_t readUint16( const char* key, uint32_t tag ) { return static_cast<uint16_t>( readInt16( key, tag ) ); }
    virtual uint32_t readUint32( const char* key, uint32_t tag ) { return static_cast<uint32_t>( readInt32( key, tag ) ); }
    virtual uint64_t readUint64( const char* key, uint32_t tag ) { return static_cast<uint64_t>( readInt64( key, tag ) ); }
  };

#define YIELD_UNMARSHALLER_PROTOTYPES \
  virtual bool readBoolean( const char* key, uint32_t tag ); \
  virtual void readBuffer( const char* key, uint32_t tag, auto_Buffer value ); \
  virtual double readDouble( const char* key, uint32_t tag ); \
  virtual int64_t readInt64( const char* key, uint32_t tag ); \
  virtual void readMap( const char* key, uint32_t tag, YIELD::Map& value ); \
  virtual void readSequence( const char* key, uint32_t tag, YIELD::Sequence& value ); \
  virtual void readString( const char* key, uint32_t tag, std::string& value ); \
  virtual void readStruct( const char* key, uint32_t tag, YIELD::Struct& value );
};

#endif
