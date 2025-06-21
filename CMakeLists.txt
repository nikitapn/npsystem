cmake_minimum_required(VERSION 4.0.1)

# third-party libraries
add_subdirectory(3rd)

set(BOOST_LIB_PREFIX "/usr/lib" CACHE STRING "Location of the boost libs")
option(OPT_BUILD_ONLY_RPC "Enable if only nprpc is needed" OFF)
option(OPT_NPRPC_SKIP_TESTS "Enable if tests should be built" OFF)

#set(OUTPUT_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
#link_directories(${OUTPUT_DIR_LIB})

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release")
endif()

message(STATUS "Building: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)

if (UNIX)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
  set(CMAKE_CXX_FLAGS_RELEASE "-O2")
endif()

if (WIN32)
    # Debugging flags
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEBUG")
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/pdb)

    # Release flags
    set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif()

if(WIN32)
  if ("${OPT_BOOST_INCLUDE_DIR}" STREQUAL "")
    message(FATAL_ERROR "-DOPT_BOOST_INCLUDE_DIR is not set")
  endif()
  if ("${OPT_BOOST_LIB_DIR}" STREQUAL "")
    message(FATAL_ERROR "-DOPT_BOOST_LIB_DIR is not set")
  endif()
  if ("${OPT_LEVELDB_INCLUDE_DIR}" STREQUAL "")
    message(FATAL_ERROR "-DOPT_LEVELDB_INCLUDE_DIR is not set")
  endif()
  if ("${OPT_LEVELDB_LIB_DIR}" STREQUAL "")
    message(FATAL_ERROR "-DOPT_LEVELDB_LIB_DIR is not set")
  endif()
  
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OPT_LEVELDB_LIB_DIR "${OPT_LEVELDB_LIB_DIR}/Release")
  else()
    set(OPT_LEVELDB_LIB_DIR "${OPT_LEVELDB_LIB_DIR}/Debug")
  endif()

  if ("${OPT_WTL_INCLUDE_DIR}" STREQUAL "")
    message(FATAL_ERROR "-DOPT_WTL_INCLUDE_DIR is not set")
  endif()

  if ("${OPT_SCINTILLA_DIR}" STREQUAL "")
    message(FATAL_ERROR "-DOPT_SCINTILLA_DIR is not set")
  endif()
  
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OPT_SCINTILLA_LIB_DIR "${OPT_SCINTILLA_DIR}/Release")
  else()
    set(OPT_SCINTILLA_LIB_DIR "${OPT_SCINTILLA_DIR}/Debug")
  endif()

  if ("${OPT_LLVM_DIR}" STREQUAL "")
    message(FATAL_ERROR "-DOPT_LLVM_DIR is not set")
  endif()
  
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(OPT_LLVM_LIB_DIR "${OPT_LLVM_DIR}/build/Release/lib")
  else()
    set(OPT_LLVM_LIB_DIR "${OPT_LLVM_DIR}/build/Debug/lib")
  endif()


  set(OPT_OPENSSL_DIR "C:\\Program Files\\OpenSSL" CACHE STRING "Path to OpenSSL directory")
 
  include_directories(
    "${OPT_BOOST_INCLUDE_DIR}"
    "${OPT_OPENSSL_DIR}/include"
    "${OPT_LEVELDB_INCLUDE_DIR}"
    "${OPT_WTL_INCLUDE_DIR}"
    "${OPT_SCINTILLA_DIR}/include"
  )

  link_directories(
    "${OPT_BOOST_LIB_DIR}"
    "${OPT_OPENSSL_DIR}/lib"
    "${OPT_LEVELDB_LIB_DIR}"
    "${OPT_SCINTILLA_LIB_DIR}"
  )

  add_definitions(
    -D_WIN32_WINNT=0x0A00
    -DBOOST_DISABLE_CURRENT_LOCATION
    -DBOOST_ASIO_DISABLE_CO_AWAIT
    -DBOOST_ASIO_DISABLE_STD_COROUTINE
    -D_SILENCE_CXX23_ALIGNED_STORAGE_DEPRECATION_WARNING
  )

  add_compile_options(/W3 /bigobj /EHsc)
else()
  add_compile_options(-Wall -Wextra -ftemplate-backtrace-limit=0 -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-deprecated-declarations)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

project(npsystem)

if (NOT OPT_NPRPC_SKIP_TESTS)
  # setting up GoogleTest
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(nplib)
add_subdirectory(nprpc)

if (NOT OPT_BUILD_ONLY_RPC)

  add_subdirectory(npc)
  add_subdirectory(npdbclient)
  add_subdirectory(npdbserver)
  add_subdirectory(npdbstatic)
  add_subdirectory(npsys)
  add_subdirectory(avr_info)
  add_subdirectory(sim)
  add_subdirectory(npserver)
  add_subdirectory(npwebserver)

  add_dependencies(npsys npc)

  if(WIN32)
  #  add_subdirectory(npcompiler)
    add_subdirectory(npsystem)
    include(avr.cmake)
  endif()
endif()
