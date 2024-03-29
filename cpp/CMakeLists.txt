cmake_minimum_required(VERSION 2.6)
PROJECT(cpp CXX)
set(BUILD_CLIENT_TESTS $ENV{BUILD_CLIENT_TESTS})

#SET (CMAKE_VERBOSE_MAKEFILE true)

add_definitions(-D_FILE_OFFSET_BITS=64)

# Set variables and required libraries.
##########################################
set(CMAKE_CXX_FLAGS "-Wall -Wno-unused-function -Wno-sign-compare")

# Set paths for required thirdparty libraries.
set(CLIENT_GOOGLE_PROTOBUF_CPP "${CMAKE_SOURCE_DIR}/thirdparty/protobuf-2.3.0")
set(CLIENT_GOOGLE_PROTOBUF_CPP_LIBRARY "${CLIENT_GOOGLE_PROTOBUF_CPP}/src/.libs/libprotobuf.a")
set(CLIENT_GOOGLE_TEST_CPP "${CMAKE_SOURCE_DIR}/thirdparty/gtest-1.5.0")
set(CLIENT_GOOGLE_TEST_CPP_LIBRARY "${CLIENT_GOOGLE_TEST_CPP}/lib/.libs/libgtest.a")
set(CLIENT_GOOGLE_TEST_CPP_MAIN "${CLIENT_GOOGLE_TEST_CPP}/lib/.libs/libgtest_main.a")

# Set required libraries.
set(LIBFUSE "fuse")
SET(LIBATTR "attr")
IF(APPLE)
  # No libattr on mac.
  SET(LIBATTR "")
  # On Apple we use the environment variable BOOST_ROOT to point to a local boost installation and we link the built boost libraries statically.
  SET(Boost_USE_STATIC_LIBS true)
  # Update 8/12/2011: Setting macosx-version-min does break mount.xtreemfs for unknown reasons - so disabled for now.
  # Tell gcc to compile 10.4 compatible files (does not work otherwise on older Macs).
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.4")
ENDIF(APPLE)

# Set required thirdparty libraries.
# Finding the boost libraries may fail. If existing, we point FIND_PACKAGE to /usr/lib64.
if(EXISTS "/usr/lib64/libboost_system.so")
  set(BOOST_LIBRARYDIR "/usr/lib64")
endif(EXISTS "/usr/lib64/libboost_system.so")
FIND_PACKAGE(Boost COMPONENTS system thread program_options regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
if (NOT Boost_FOUND)
  message(FATAL_ERROR "The boost library was not found on your system. If needed, you can also download and compile it on your own. After compiling boost locally, set the the environment variable BOOST_ROOT to the boost base directory, e.g. export BOOST_ROOT=/Users/xyz/boost_1_47_0.")
endif(NOT Boost_FOUND)


SET(REQUIRED_STATIC_LIBRARIES ${CLIENT_GOOGLE_PROTOBUF_CPP_LIBRARY})
if(BUILD_CLIENT_TESTS)
  SET(REQUIRED_STATIC_LIBRARIES ${REQUIRED_STATIC_LIBRARIES} ${CLIENT_GOOGLE_TEST_CPP_LIBRARY} ${CLIENT_GOOGLE_TEST_CPP_MAIN})
endif(BUILD_CLIENT_TESTS)

foreach (STATIC_LIB_FULL_PATH ${REQUIRED_STATIC_LIBRARIES})
  if(NOT EXISTS ${STATIC_LIB_FULL_PATH})
    message(FATAL_ERROR "The file ${STATIC_LIB_FULL_PATH} was not found. Please run 'make client' or 'make' from the base directory in order to build the required static libraries.")
  endif()
endforeach(STATIC_LIB_FULL_PATH)

CMAKE_POLICY(SET CMP0003 OLD)
ADD_LIBRARY(protobuf STATIC IMPORTED)
SET_PROPERTY(TARGET protobuf PROPERTY IMPORTED_LOCATION ${CLIENT_GOOGLE_PROTOBUF_CPP_LIBRARY})
include_directories(${CLIENT_GOOGLE_PROTOBUF_CPP}/src)

ADD_LIBRARY(gtest STATIC IMPORTED)
ADD_LIBRARY(gtest_main STATIC IMPORTED)
SET_PROPERTY(TARGET gtest PROPERTY IMPORTED_LOCATION ${CLIENT_GOOGLE_TEST_CPP_LIBRARY})
SET_PROPERTY(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${CLIENT_GOOGLE_TEST_CPP_MAIN})
include_directories(${CLIENT_GOOGLE_TEST_CPP}/include)

# Check if this is a 64 Bit system
if(UNIX AND NOT WIN32)
  if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    if(APPLE)
      set(LIBFUSE "fuse_ino64")
    endif(APPLE)
  endif(CMAKE_SIZEOF_VOID_P MATCHES "8")
endif(UNIX AND NOT WIN32)

SET(REQUIRED_LIBRARIES ${LIBFUSE} ${LIBATTR} "crypto" "ssl" "pthread")
foreach(LIB ${REQUIRED_LIBRARIES})
  #message(STATUS "checking for library: ${LIB}")
  # For unknown reasons, setting "FIND_LIBRARY_USE_LIB64_PATHS" does not fix the issue that libraries in /usr/lib64/ are not found under openSuse.
  # Therefore we just specify a list of possible library directories.
  find_library(FOUND${LIB} ${LIB} PATHS "/lib64" "/usr/lib64")
  if (NOT FOUND${LIB})
    message(FATAL_ERROR "The required library '${LIB}' was not found. Please install it on your system first.")
  #else()
  #  message(STATUS "Result of find_library: ${FOUND${LIB}}")
  endif(NOT FOUND${LIB})
endforeach(LIB)

# Define building of binaries.
##########################################
MESSAGE(STATUS "Configuring XtreemFS client (libxtreemfs, FuseAdapter(mount.xtreemfs) and Volume Tools ({mkfs,rmfs,lsfs}.xtreemfs).")

INCLUDE_DIRECTORIES(include generated)
file(GLOB_RECURSE SRCS_RPC src/rpc/*.cpp)
file(GLOB_RECURSE SRCS_UTIL src/util/*.cpp)
file(GLOB_RECURSE SRCS_GENERATED generated/*.cc)
file(GLOB_RECURSE SRCS_XTREEMFS src/libxtreemfs/*.cpp)
add_library(xtreemfs ${SRCS_RPC} ${SRCS_UTIL} ${SRCS_GENERATED} ${SRCS_XTREEMFS})

ADD_EXECUTABLE(example_libxtreemfs src/example_libxtreemfs/example_libxtreemfs.cpp)
TARGET_LINK_LIBRARIES(example_libxtreemfs xtreemfs protobuf crypto pthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ssl)

file(GLOB_RECURSE SRCS_FUSE_ADAPTER src/fuse/*.cpp)
file(GLOB_RECURSE SRCS_XTFS_UTIL src/xtfsutil/xtfsutil_server.cpp)
file(GLOB_RECURSE SRCS_JSONCPP src/json/*.cpp)
ADD_EXECUTABLE(mount.xtreemfs ${SRCS_FUSE_ADAPTER} ${SRCS_XTFS_UTIL} ${SRCS_JSONCPP})
TARGET_LINK_LIBRARIES(mount.xtreemfs xtreemfs protobuf crypto pthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ssl ${LIBFUSE})

file(GLOB_RECURSE SRCS_MKFS src/mkfs.xtreemfs/*.cpp)
ADD_EXECUTABLE(mkfs.xtreemfs ${SRCS_MKFS})
TARGET_LINK_LIBRARIES(mkfs.xtreemfs xtreemfs protobuf crypto pthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ssl)

file(GLOB_RECURSE SRCS_RMFS src/rmfs.xtreemfs/*.cpp)
ADD_EXECUTABLE(rmfs.xtreemfs ${SRCS_RMFS})
TARGET_LINK_LIBRARIES(rmfs.xtreemfs xtreemfs protobuf crypto pthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ssl)

file(GLOB_RECURSE SRCS_LSFS src/lsfs.xtreemfs/*.cpp)
ADD_EXECUTABLE(lsfs.xtreemfs ${SRCS_LSFS})
TARGET_LINK_LIBRARIES(lsfs.xtreemfs xtreemfs protobuf crypto pthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ssl)

ADD_EXECUTABLE(xtfsutil src/xtfsutil/xtfsutil.cpp ${SRCS_JSONCPP})
TARGET_LINK_LIBRARIES(xtfsutil ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_REGEX_LIBRARY} ${LIBATTR})

# Define building of unittests.
##########################################
if(BUILD_CLIENT_TESTS)
  # enable_testing() will provide a target "test".
  enable_testing()

  MESSAGE(STATUS "Configuring libxtreemfs unittests.")

  file(GLOB TESTS test/*)
  foreach (testdir ${TESTS})
    if(IS_DIRECTORY ${testdir})
      get_filename_component(testname ${testdir} NAME)
      set(testname "test_${testname}")
      file(GLOB_RECURSE SRCS_TESTS ${testdir}/*.cpp)
      if (SRCS_TESTS)
        MESSAGE(STATUS "\tConfiguring test: ${testname}.")
        add_executable(${testname} ${SRCS_TESTS})
        TARGET_LINK_LIBRARIES(${testname} gtest_main gtest xtreemfs protobuf crypto pthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ssl)
        ADD_TEST(${testname} ${testname})
      endif()
    endif(IS_DIRECTORY ${testdir})
  endforeach(testdir)
endif(BUILD_CLIENT_TESTS)
