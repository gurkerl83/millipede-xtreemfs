// Copyright (c) 2010 Minor Gordon
// All rights reserved
// 
// This source file is part of the yidl project.
// It is licensed under the New BSD license:
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of the yidl project nor the
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


#ifndef _YIDL_H_
#define _YIDL_H_

#define __STDC_LIMIT_MACROS

#ifdef _WIN32
// msstdint.h
// ISO C9x  compliant stdint.h for Microsoft Visual Studio
// Based on ISO/IEC 9899:TC2 Committee draft (May 6, 2005) WG14/N1124
//
//  Copyright (c) 2006-2008 Alexander Chemeris
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//   3. The name of the author may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////////

#include <limits.h>

// For Visual Studio 6 in C++ mode wrap <wchar.h> include with 'extern "C++" {}'
// or compiler give many errors like this:
//   error C2733: second C linkage of overloaded function 'wmemchr' not allowed
#if (_MSC_VER < 1300) && defined(__cplusplus)
   extern "C++" {
#endif
#     include <wchar.h>
#if (_MSC_VER < 1300) && defined(__cplusplus)
   }
#endif

// Define _W64 macros to mark types changing their size, like intptr_t.
#ifndef _W64
#  if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#     define _W64 __w64
#  else
#     define _W64
#  endif
#endif


// 7.18.1 Integer types

// 7.18.1.1 Exact-width integer types
typedef __int8            int8_t;
typedef __int16           int16_t;
typedef __int32           int32_t;
typedef __int64           int64_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
typedef unsigned __int64  uint64_t;

// 7.18.1.2 Minimum-width integer types
typedef int8_t    int_least8_t;
typedef int16_t   int_least16_t;
typedef int32_t   int_least32_t;
typedef int64_t   int_least64_t;
typedef uint8_t   uint_least8_t;
typedef uint16_t  uint_least16_t;
typedef uint32_t  uint_least32_t;
typedef uint64_t  uint_least64_t;

// 7.18.1.3 Fastest minimum-width integer types
typedef int8_t    int_fast8_t;
typedef int16_t   int_fast16_t;
typedef int32_t   int_fast32_t;
typedef int64_t   int_fast64_t;
typedef uint8_t   uint_fast8_t;
typedef uint16_t  uint_fast16_t;
typedef uint32_t  uint_fast32_t;
typedef uint64_t  uint_fast64_t;

// 7.18.1.4 Integer types capable of holding object pointers
#ifdef _WIN64 // [
   typedef __int64           intptr_t;
   typedef unsigned __int64  uintptr_t;
#else // _WIN64 ][
   typedef _W64 int               intptr_t;
   typedef _W64 unsigned int      uintptr_t;
#endif // _WIN64 ]

// 7.18.1.5 Greatest-width integer types
typedef int64_t   intmax_t;
typedef uint64_t  uintmax_t;


// 7.18.2 Limits of specified-width integer types

#if !defined(__cplusplus) || defined(__STDC_LIMIT_MACROS) // [   See footnote 220 at page 257 and footnote 221 at page 259

// 7.18.2.1 Limits of exact-width integer types
#define INT8_MIN     ((int8_t)_I8_MIN)
#define INT8_MAX     _I8_MAX
#define INT16_MIN    ((int16_t)_I16_MIN)
#define INT16_MAX    _I16_MAX
#define INT32_MIN    ((int32_t)_I32_MIN)
#define INT32_MAX    _I32_MAX
#define INT64_MIN    ((int64_t)_I64_MIN)
#define INT64_MAX    _I64_MAX
#define UINT8_MAX    _UI8_MAX
#define UINT16_MAX   _UI16_MAX
#define UINT32_MAX   _UI32_MAX
#define UINT64_MAX   _UI64_MAX

// 7.18.2.2 Limits of minimum-width integer types
#define INT_LEAST8_MIN    INT8_MIN
#define INT_LEAST8_MAX    INT8_MAX
#define INT_LEAST16_MIN   INT16_MIN
#define INT_LEAST16_MAX   INT16_MAX
#define INT_LEAST32_MIN   INT32_MIN
#define INT_LEAST32_MAX   INT32_MAX
#define INT_LEAST64_MIN   INT64_MIN
#define INT_LEAST64_MAX   INT64_MAX
#define UINT_LEAST8_MAX   UINT8_MAX
#define UINT_LEAST16_MAX  UINT16_MAX
#define UINT_LEAST32_MAX  UINT32_MAX
#define UINT_LEAST64_MAX  UINT64_MAX

// 7.18.2.3 Limits of fastest minimum-width integer types
#define INT_FAST8_MIN    INT8_MIN
#define INT_FAST8_MAX    INT8_MAX
#define INT_FAST16_MIN   INT16_MIN
#define INT_FAST16_MAX   INT16_MAX
#define INT_FAST32_MIN   INT32_MIN
#define INT_FAST32_MAX   INT32_MAX
#define INT_FAST64_MIN   INT64_MIN
#define INT_FAST64_MAX   INT64_MAX
#define UINT_FAST8_MAX   UINT8_MAX
#define UINT_FAST16_MAX  UINT16_MAX
#define UINT_FAST32_MAX  UINT32_MAX
#define UINT_FAST64_MAX  UINT64_MAX

// 7.18.2.4 Limits of integer types capable of holding object pointers
#ifdef _WIN64 // [
#  define INTPTR_MIN   INT64_MIN
#  define INTPTR_MAX   INT64_MAX
#  define UINTPTR_MAX  UINT64_MAX
#else // _WIN64 ][
#  define INTPTR_MIN   INT32_MIN
#  define INTPTR_MAX   INT32_MAX
#  define UINTPTR_MAX  UINT32_MAX
#endif // _WIN64 ]

// 7.18.2.5 Limits of greatest-width integer types
#define INTMAX_MIN   INT64_MIN
#define INTMAX_MAX   INT64_MAX
#define UINTMAX_MAX  UINT64_MAX

// 7.18.3 Limits of other integer types

#ifdef _WIN64 // [
#  define PTRDIFF_MIN  _I64_MIN
#  define PTRDIFF_MAX  _I64_MAX
#else  // _WIN64 ][
#  define PTRDIFF_MIN  _I32_MIN
#  define PTRDIFF_MAX  _I32_MAX
#endif  // _WIN64 ]

#define SIG_ATOMIC_MIN  INT_MIN
#define SIG_ATOMIC_MAX  INT_MAX

#ifndef SIZE_MAX // [
#  ifdef _WIN64 // [
#     define SIZE_MAX  _UI64_MAX
#  else // _WIN64 ][
#     define SIZE_MAX  _UI32_MAX
#  endif // _WIN64 ]
#endif // SIZE_MAX ]

// WCHAR_MIN and WCHAR_MAX are also defined in <wchar.h>
#ifndef WCHAR_MIN // [
#  define WCHAR_MIN  0
#endif  // WCHAR_MIN ]
#ifndef WCHAR_MAX // [
#  define WCHAR_MAX  _UI16_MAX
#endif  // WCHAR_MAX ]

#define WINT_MIN  0
#define WINT_MAX  _UI16_MAX

#endif // __STDC_LIMIT_MACROS ]


// 7.18.4 Limits of other integer types

#if !defined(__cplusplus) || defined(__STDC_CONSTANT_MACROS) // [   See footnote 224 at page 260

// 7.18.4.1 Macros for minimum-width integer constants

#define INT8_C(val)  val##i8
#define INT16_C(val) val##i16
#define INT32_C(val) val##i32
#define INT64_C(val) val##i64

#define UINT8_C(val)  val##ui8
#define UINT16_C(val) val##ui16
#define UINT32_C(val) val##ui32
#define UINT64_C(val) val##ui64

// 7.18.4.2 Macros for greatest-width integer constants
#define INTMAX_C   INT64_C
#define UINTMAX_C  UINT64_C

#endif // __STDC_CONSTANT_MACROS ]

#else // !_WIN32

#include <stdint.h>
#include <sys/uio.h> // For struct iovec

#endif // _WIN32

#include <cstring>
#include <ostream>
#include <string>


#if defined(_WIN64)
extern "C"
{
  __declspec( dllimport ) void __stdcall DebugBreak();

  __int64 _InterlockedCompareExchange64
  (
    volatile __int64* current_value,
    __int64 new_value,
    __int64 old_value
  );

  __int64 _InterlockedIncrement64( volatile __int64* current_value );
  __int64 _InterlockedDecrement64( volatile __int64* current_value );
}
#elif defined(_WIN32)
extern "C"
{
  __declspec( dllimport ) void __stdcall DebugBreak();

  __declspec( dllimport ) long __stdcall
  InterlockedCompareExchange
  (
    volatile long* current_value,
    long new_value,
    long old_value
  );

  __declspec( dllimport ) long __stdcall
  InterlockedIncrement
  (
    volatile long* current_value
  );

  __declspec( dllimport ) long __stdcall
  InterlockedDecrement
  (
    volatile long* current_value
  );
}
#endif // _WIN32


#ifdef _WIN32

struct iovec // a WSABUF on 32-bit systems
{
  size_t iov_len; // the WSABUF .len is actually a ULONG,
                  // which is != size_t on Win64
                  // That means that we have to copy and
                  // truncate an iovec to a WSABUF on Win64.
                  // That's easier (in terms of warnings, use of sizeof, etc.)
                  // than making this a uint32_t, however.
  void* iov_base;
};

#else // !_WIN32

static inline void DebugBreak()
{
  *reinterpret_cast<int*>( 0 ) = 0xabadcafe;
}

inline void memcpy_s
(
  void* dest,
  size_t dest_size,
  const void* src,
  size_t count
)
{
  memcpy( dest, src, count );
}

#ifndef __linux__
inline size_t strnlen( const char* s, size_t maxlen )
{
  return strlen( s );
}
#endif

#if defined(__sun)
#include <atomic.h>
#elif defined(__arm__)
// gcc atomic builtins are not defined on ARM
#elif defined(__GNUC__) && \
      ( ( __GNUC__ == 4 && __GNUC_MINOR__ >= 1 ) || __GNUC__ > 4 )
#define __HAVE_GNUC_ATOMIC_BUILTINS 1
#endif

#endif // _WIN32


#define YIDL_RUNTIME_OBJECT_PROTOTYPES( type_name, type_id ) \
    type_name & incRef() { return ::yidl::runtime::Object::incRef( *this ); } \
    const static uint32_t __type_id = static_cast<uint32_t>( type_id ); \
    virtual uint32_t get_type_id() const { return __type_id; } \
    const char* get_type_name() const { return #type_name; }

#define YIDL_RUNTIME_OBJECT_TYPE_ID( type ) type::__type_id


namespace yidl
{
  namespace runtime
  {
#if defined(_WIN64)
    typedef __int64 atomic_t;
#elif defined(_WIN32)
    typedef long atomic_t;
#elif defined(__LLP64__) || defined(__LP64__)
    typedef int64_t atomic_t;
#else
    typedef int32_t atomic_t;
#endif

    static inline atomic_t atomic_cas
    (
      volatile atomic_t* current_value,
      atomic_t new_value,
      atomic_t old_value
    )
    {
#if defined(_WIN64)
      return _InterlockedCompareExchange64( current_value, new_value, old_value );
#elif defined(_WIN32)
      return InterlockedCompareExchange( current_value, new_value, old_value );
#elif defined(__sun)
      return atomic_cas_32( current_value, old_value, new_value );
#elif defined(__HAVE_GNUC_ATOMIC_BUILTINS)
      return __sync_val_compare_and_swap( current_value, old_value, new_value );
#elif defined(__arm__)
#if __ARM_ARCH__ >= 6
      atomic_t prev;
      asm volatile( "@ atomic_cmpxchg\n"
                    "ldrex  %1, [%2]\n"
                    "mov    %0, #0\n"
                    "teq    %1, %3\n"
                    "strexeq %0, %4, [%2]\n"
                     : "=&r" ( prev), "=&r" ( old_value )
                     : "r" ( current_value ), "Ir" ( old_value ), "r" ( new_value )
                     : "cc" );
      return prev;
#else // ARM architectures < 6 are uniprocessor only
      if ( *current_value == old_value )
      {
        *current_value = new_value;
        return old_value;
      }
      else
        return *current_value;
#endif
#elif defined(__i386__)
      atomic_t prev;
      asm volatile(  "lock\n"
              "cmpxchgl %1,%2\n"
            : "=a" ( prev )
                  : "r" ( new_value ), "m" ( *current_value ) , "0" ( old_value )
                  : "memory"
                );
      return prev;
#elif defined(__ppc__)
      atomic_t prev;
      asm volatile(  "          \n\
              1:  ldarx   %0,0,%2 \n\
              cmpd    0,%0,%3 \n\
              bne     2f    \n\
              stdcx.  %4,0,%2 \n\
              bne-    1b    \n\
              sync\n"
              "2:"
            : "=&r" ( prev ), "=m" ( *current_value )
                  : "r" ( current_value ), "r" ( old_value ), "r" ( new_value ),
                    "m" ( *current_value )
                  : "cc", "memory"
                );
      return prev;
#elif defined(__x86_64__)
      atomic_t prev;
      asm volatile(  "lock\n"
              "cmpxchgq %1,%2\n"
            : "=a" ( prev )
                  : "r" ( new_value ), "m" ( *current_value ) , "0" ( old_value )
                  : "memory"
                );
      return prev;
#endif
    }

    static inline atomic_t atomic_dec( volatile atomic_t* current_value )
    {
#if defined(_WIN64)
      return _InterlockedDecrement64( current_value );
#elif defined(_WIN32)
      return InterlockedDecrement( current_value );
#elif defined(__sun)
      return atomic_dec_32_nv( current_value );
#elif defined(__HAVE_GNUC_ATOMIC_BUILTINS)
      return __sync_sub_and_fetch( current_value, 1 );
#else
      atomic_t old_value, new_value;

      do
      {
        old_value = *current_value;
        new_value = old_value - 1;
      }
      while ( atomic_cas( current_value, new_value, old_value ) != old_value );

      return new_value;
#endif
    }

    static inline atomic_t atomic_inc( volatile atomic_t* current_value )
    {
#if defined(_WIN64)
      return _InterlockedIncrement64( current_value );
#elif defined(_WIN32)
      return InterlockedIncrement( current_value );
#elif defined(__sun)
      return atomic_inc_32_nv( current_value );
#elif defined(__HAVE_GNUC_ATOMIC_BUILTINS)
      return __sync_add_and_fetch( current_value, 1 );
#else
      atomic_t old_value, new_value;

      do
      {
        old_value = *current_value;
        new_value = old_value + 1;
      }
      while ( atomic_cas( current_value, new_value, old_value ) != old_value );

      return new_value;
#endif
    }


    class Marshaller;
    class Sequence;
    class Struct;
    class Unmarshaller;


    class Object
    {
    public:
      Object() : refcnt( 1 )
      { }

      static inline void decRef( Object& object )
      {
#ifdef YIDL_DEBUG_REFERENCE_COUNTING
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
#ifdef YIDL_DEBUG_REFERENCE_COUNTING
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
      volatile atomic_t refcnt;
    };


    // auto_Object isike auto_ptr, but using Object::decRef instead of delete;
    // an operator delete( void* ) on Object doesn't work, because the object
    // is destructed before that call
    template <class ObjectType = Object>
    class auto_Object
    {
    public:
      auto_Object()
        : object( 0 )
      { }

      auto_Object( ObjectType* object )
        : object( object )
      { }

      auto_Object( ObjectType& object )
        : object( &object )
      { }

      auto_Object( const auto_Object<ObjectType>& other )
      {
  	    object = Object::incRef( other.object );
      }

      ~auto_Object()
      {
        Object::decRef( object );
      }

      inline ObjectType* get() const
      {
        return object;
      }

      auto_Object& operator=( const auto_Object<ObjectType>& other )
      {
        Object::decRef( this->object );
        object = Object::incRef( other.object );
        return *this;
      }

      auto_Object& operator=( ObjectType* object )
      {
        Object::decRef( this->object );
        this->object = object;
        return *this;
      }

      inline bool operator==( const auto_Object<ObjectType>& other ) const
      {
        return object == other.object;
      }

      inline bool operator==( const ObjectType* other ) const
      {
        return object == other;
      }

      inline bool operator!=( const ObjectType* other ) const
      {
        return object != other;
      }

      // Creates sneaky bugs
      // operator ObjectType*() const { return object; }

      inline ObjectType* operator->() const
      {
        return get();
      }

      inline ObjectType& operator*() const
      {
        return *get();
      }

      inline ObjectType* release()
      {
        ObjectType* temp_object = object;
        object = 0;
        return temp_object;
      }

      inline void reset( ObjectType* object )
      {
        Object::decRef( this->object );
        this->object = object;
      }

    private:
      ObjectType* object;
    };


    class Buffer : public Object
    {
    public:
      virtual ~Buffer() { }

      // capacity: the number of bytes available in the buffer >= size
      virtual size_t capacity() const = 0;

      inline bool empty() const { return size() == 0; }

      // get: copy out of the buffer, advancing position
      virtual size_t get( void* into_buffer, size_t into_buffer_len ) = 0;

      // Casts to get at the underlying buffer;
      // may not be implemented by some buffers
      operator char*() const
      {
        return static_cast<char*>( static_cast<void*>( *this ) );
      }

      operator unsigned char*() const
      {
        return static_cast<unsigned char*>( static_cast<void*>( *this ) );
      }

      virtual operator void*() const = 0;

      bool operator==( const Buffer& other ) const
      {
        if ( size() == other.size() )
        {
          void* this_base = static_cast<void*>( *this );
          void* other_base = static_cast<void*>( other );
          if ( this_base != NULL && other_base != NULL )
            return memcmp( this_base, other_base, size() ) == 0;
          else
            return false;
        }
        else
          return false;
      }

      // position: get and set the get() position
      size_t position() const { return _position; }

      void position( size_t new_position )
      {
        if ( new_position < size() )
          _position = new_position;
        else
          _position = size();
      }

      // put: append bytes to the buffer, increases size but not position
      size_t put( const char* from_string )
      {
        return put( from_string, strlen( from_string ) );
      }

      size_t put( const std::string& from_string )
      {
        return put( from_string.c_str(), from_string.size() );
      }

      virtual size_t put( const void* from_buf, size_t from_buf_len ) = 0;

      // size: the number of filled bytes, <= capacity
      virtual size_t size() const = 0;

      // Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( Buffer, 1 );

    protected:
      Buffer()
      {
        _position = 0;
      }

    private:
      size_t _position;
    };

    typedef auto_Object<Buffer> auto_Buffer;


    class FixedBuffer : public Buffer
    {
    public:
      bool operator==( const FixedBuffer& other ) const
      {
        return iov.iov_len == other.iov.iov_len &&
               memcmp( iov.iov_base, other.iov.iov_base, iov.iov_len ) == 0;
      }

      // Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( FixedBuffer, 2 );

      // Buffer
      size_t get( void* into_buffer, size_t into_buffer_len )
      {
        if ( iov.iov_len - position() < into_buffer_len )
          into_buffer_len = iov.iov_len - position();

        if ( into_buffer != NULL )
        {
          memcpy_s
          (
            into_buffer,
            into_buffer_len,
            static_cast<uint8_t*>( iov.iov_base ) + position(),
            into_buffer_len
          );
        }

        position( position() + into_buffer_len );

        return into_buffer_len;
      }

      size_t capacity() const { return _capacity; }
      operator iovec() const { return iov; }
      operator void*() const { return iov.iov_base; }

      virtual size_t put( const void* from_buffer, size_t from_buffer_len )
      {
        if ( capacity() - iov.iov_len < from_buffer_len )
          from_buffer_len = capacity() - iov.iov_len;

        if ( from_buffer != NULL && from_buffer != iov.iov_base )
        {
          memcpy_s
          (
            static_cast<uint8_t*>( iov.iov_base ) + iov.iov_len,
            capacity() - iov.iov_len,
            from_buffer,
            from_buffer_len
          );
        }

        iov.iov_len += from_buffer_len;
        return from_buffer_len;
      }

      size_t size() const { return iov.iov_len; }

    protected:
      FixedBuffer( size_t capacity )
        : _capacity( capacity )
      {
        iov.iov_len = 0;
      }

      struct iovec iov;

    private:
      size_t _capacity;
    };


    class HeapBuffer : public FixedBuffer
    {
    public:
      HeapBuffer( size_t capacity )
        : FixedBuffer( capacity )
      {
        iov.iov_base = new uint8_t[capacity];
      }

      virtual ~HeapBuffer()
      {
        delete [] static_cast<uint8_t*>( iov.iov_base );
      }

      // Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( HeapBuffer, 4 );
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

      virtual void writeBoolean
      (
        const char* key,
        uint32_t tag,
        bool value
      ) = 0;

      virtual void writeBuffer
      (
        const char* key,
        uint32_t tag,
        auto_Object<Buffer> value
      ) = 0;

      virtual void writeFloat
      (
        const char* key,
        uint32_t tag,
        float value )
      {
        writeDouble( key, tag, value );
      }

      virtual void writeDouble
      (
        const char* key,
        uint32_t tag,
        double value
      ) = 0;

      virtual void writeInt8
      (
        const char* key,
        uint32_t tag,
        int8_t value
      )
      {
        writeInt16( key, tag, value );
      }

      virtual void writeInt16
      (
        const char* key,
        uint32_t tag,
        int16_t value
      )
      {
        writeInt32( key, tag, value );
      }

      virtual void writeInt32
      (
        const char* key,
        uint32_t tag,
        int32_t value
      )
      {
        writeInt64( key, tag, value );
      }

      virtual void writeInt64
      (
        const char* key,
        uint32_t tag,
        int64_t value
      ) = 0;

      virtual void writeMap
      (
        const char* key,
        uint32_t tag,
        const Map& value
      ) = 0;

      virtual void writeSequence
      (
        const char* key,
        uint32_t tag,
        const Sequence& value
      ) = 0;

      virtual void writeStruct
      (
        const char* key,
        uint32_t tag,
        const Struct& value
      ) = 0;

      virtual void writeString
      (
        const char* key,
        uint32_t tag,
        const std::string& value
      )
      {
        writeString( key, tag, value.c_str(), value.size() );
      }

      virtual void writeString
      (
        const char* key,
        uint32_t tag,
        const char* value
      )
      {
        writeString( key, tag, value, strnlen( value, UINT16_MAX ) );
      }

      virtual void writeString
      (
        const char* key,
        uint32_t tag,
        const char* value,
        size_t value_len
      ) = 0;

      virtual void writeUint8
      (
        const char* key,
        uint32_t tag,
        uint8_t value
      )
      {
        writeInt8( key, tag, static_cast<int8_t>( value ) );
      }

      virtual void writeUint16
      (
        const char* key,
        uint32_t tag,
        uint16_t value
      )
      {
        writeInt16( key, tag, static_cast<int16_t>( value ) );
      }

      virtual void writeUint32
      (
        const char* key,
        uint32_t tag,
        uint32_t value
      )
      {
        writeInt32( key, tag, static_cast<int32_t>( value ) );
      }

      virtual void writeUint64
      (
        const char* key,
        uint32_t tag,
        uint64_t value
      )
      {
        writeInt64( key, tag, static_cast<int64_t>( value ) );
      }
    };

#define YIDL_MARSHALLER_PROTOTYPES \
    virtual void writeBoolean( const char* key, uint32_t tag, bool value ); \
    virtual void writeBuffer \
    ( \
      const char* key, \
      uint32_t tag, \
     ::yidl::runtime::auto_Buffer value \
    ); \
    virtual void writeDouble( const char* key, uint32_t tag, double value ); \
    virtual void writeInt64( const char* key, uint32_t tag, int64_t value ); \
    virtual void writeMap \
    ( \
      const char* key, \
      uint32_t tag, \
      const ::yidl::runtime::Map& value \
    ); \
    virtual void writeSequence \
    ( \
      const char* key, \
      uint32_t tag, \
      const ::yidl::runtime::Sequence& value \
    ); \
    virtual void writeString \
    ( \
      const char* key, \
      uint32_t tag, \
      const char* value, \
      size_t value_len \
    ); \
    virtual void writeStruct \
    ( \
      const char* key, \
      uint32_t tag, \
      const ::yidl::runtime::Struct& value \
    );


    class PrettyPrinter : public Marshaller
    {
    public:
      PrettyPrinter( std::ostream& os )
        : os( os )
      { }

      PrettyPrinter& operator=( const PrettyPrinter& )
      {
        return *this;
      }

      // Marshaller
      void writeBoolean( const char*, uint32_t, bool value )
      {
        if ( value )
          os << "true, ";
        else
          os << "false, ";
      }

      void writeBuffer( const char*, uint32_t, auto_Buffer )
      { }

      void writeDouble( const char*, uint32_t, double value )
      {
        os << value << ", ";
      }

      void writeInt64( const char*, uint32_t, int64_t value )
      {
        os << value << ", ";
      }

      void writeMap( const char*, uint32_t, const Map& value )
      {
        os << value.get_type_name() << "( ";
        value.marshal( *this );
        os << " ), ";
      }

      void writeSequence( const char*, uint32_t, const Sequence& value );

      void writeString
      (
        const char*,
        uint32_t,
        const char* value,
        size_t value_len
      )
      {
        os.write( value, value_len );
        os << ", ";
      }

      void writeStruct( const char*, uint32_t, const Struct& value );

    private:
      std::ostream& os;
    };


    class Sequence : public Object
    {
    public:
      virtual size_t get_size() const = 0;
    };


    inline void PrettyPrinter::writeSequence
    (
      const char*,
      uint32_t,
      const Sequence& value
    )
    {
      os << "[ ";
      value.marshal( *this );
      os << " ], ";
    }


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
      YIDL_RUNTIME_OBJECT_PROTOTYPES( StackBuffer, 5 );

    private:
      uint8_t _stack_buffer[Capacity];
    };


    class StringBuffer : public Buffer
    {
    public:
      StringBuffer()
      { }

      StringBuffer( size_t capacity )
      {
        _string.reserve( capacity );
      }

      StringBuffer( const std::string& str )
        : _string( str )
      { }

      StringBuffer( const char* str )
       : _string( str )
      { }

      StringBuffer( const char* str, size_t str_len )
        : _string( str, str_len )
      { }

      const char* c_str() const { return _string.c_str(); }
      operator std::string&() { return _string; }
      operator const std::string&() const { return _string; }
      operator void*() const { return const_cast<char*>( _string.c_str() ); }

      bool operator==( const StringBuffer& other ) const
      {
        return _string == other._string;
      }

      bool operator==( const char* other ) const
      {
        return _string == other;
      }

      // Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( StringBuffer, 6 );

      // Buffer
      size_t capacity() const { return _string.capacity(); }

      size_t get( void* into_buffer, size_t into_buffer_len )
      {
        if ( size() - position() < into_buffer_len )
          into_buffer_len = size() - position();

        memcpy_s
        (
          into_buffer,
          into_buffer_len,
          _string.c_str() + position(),
          into_buffer_len
        );

        position( position() + into_buffer_len );

        return into_buffer_len;
      }

      size_t put( const void* from_buffer, size_t from_buffer_len )
      {
        _string.append
        (
          static_cast<const char*>( from_buffer ),
          from_buffer_len
        );

        return from_buffer_len;
      }

      size_t size() const { return _string.size(); }

    private:
      std::string _string;
    };

    typedef auto_Object<StringBuffer> auto_StringBuffer;


    class StringLiteralBuffer : public FixedBuffer
    {
    public:
      StringLiteralBuffer( const char* string_literal )
        : FixedBuffer( strnlen( string_literal, UINT16_MAX ) )
      {
        iov.iov_base = const_cast<char*>( string_literal );
        iov.iov_len = capacity();
      }

      StringLiteralBuffer
      (
        const char* string_literal,
        size_t string_literal_len
      )
        : FixedBuffer( string_literal_len )
      {
        iov.iov_base = const_cast<char*>( string_literal );
        iov.iov_len = string_literal_len;
      }

      StringLiteralBuffer
      (
        const void* string_literal,
        size_t string_literal_len
      )
        : FixedBuffer( string_literal_len )
      {
        iov.iov_base = const_cast<void*>( string_literal );
        iov.iov_len = string_literal_len;
      }

      // Object
      YIDL_RUNTIME_OBJECT_PROTOTYPES( StringLiteralBuffer, 7 );

      // Buffer
      size_t put( const void*, size_t ) { return 0; }
    };


    class Struct : public Object
    { };

    typedef auto_Object<Struct> auto_Struct;


    inline void PrettyPrinter::writeStruct
    (
      const char*,
      uint32_t,
      const Struct& value
    )
    {
      os << value.get_type_name() << "( ";
      value.marshal( *this );
      os << " ), ";
    }


    class Unmarshaller
    {
    public:
      virtual ~Unmarshaller() { }

      virtual bool readBoolean( const char* key, uint32_t tag ) = 0;

      virtual void readBuffer
      (
        const char* key,
        uint32_t tag,
        auto_Buffer value
      ) = 0;

      virtual double readDouble( const char* key, uint32_t tag ) = 0;

      virtual float readFloat( const char* key, uint32_t tag )
      {
        return static_cast<float>( readDouble( key, tag ) );
      }

      virtual int8_t readInt8( const char* key, uint32_t tag )
      {
        return static_cast<int8_t>( readInt16( key, tag ) );
      }

      virtual int16_t readInt16( const char* key, uint32_t tag )
      {
        return static_cast<int16_t>( readInt32( key, tag ) );
      }

      virtual int32_t readInt32( const char* key, uint32_t tag )
      {
        return static_cast<int32_t>( readInt64( key, tag ) );
      }

      virtual int64_t readInt64( const char* key, uint32_t tag ) = 0;

      virtual void readMap( const char* key, uint32_t tag, Map& value ) = 0;

      virtual void readSequence
      (
        const char* key,
        uint32_t tag,
        Sequence& value
      ) = 0;

      virtual void readString
      (
        const char* key,
        uint32_t tag,
        std::string& value
      ) = 0;

      virtual void readStruct
      (
        const char* key,
        uint32_t tag,
        Struct& value
      ) = 0;

      virtual uint8_t readUint8( const char* key, uint32_t tag )
      {
        return static_cast<uint8_t>( readInt8( key, tag ) );
      }

      virtual uint16_t readUint16( const char* key, uint32_t tag )
      {
        return static_cast<uint16_t>( readInt16( key, tag ) );
      }

      virtual uint32_t readUint32( const char* key, uint32_t tag )
      {
        return static_cast<uint32_t>( readInt32( key, tag ) );
      }

      virtual uint64_t readUint64( const char* key, uint32_t tag )
      {
        return static_cast<uint64_t>( readInt64( key, tag ) );
      }
    };

#define YIDL_UNMARSHALLER_PROTOTYPES \
    virtual bool readBoolean( const char* key, uint32_t tag ); \
    virtual void readBuffer \
    ( \
      const char* key, \
      uint32_t tag, \
      ::yidl::runtime::auto_Buffer value \
    ); \
    virtual double readDouble( const char* key, uint32_t tag ); \
    virtual int64_t readInt64( const char* key, uint32_t tag ); \
    virtual void readMap \
    ( \
      const char* key, \
      uint32_t tag, \
      ::yidl::runtime::Map& value \
    ); \
    virtual void readSequence \
    ( \
      const char* key, \
      uint32_t tag, \
      ::yidl::runtime::Sequence& value \
    ); \
    virtual void readString \
    ( \
      const char* key, \
      uint32_t tag, \
      std::string& value \
    ); \
    virtual void readStruct \
    ( \
      const char* key, \
      uint32_t tag, \
      ::yidl::runtime::Struct& value \
    );
  };
};

#endif
