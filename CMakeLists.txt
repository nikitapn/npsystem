cmake_minimum_required(VERSION 3.28.1)

#set(OUTPUT_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
#link_directories(${OUTPUT_DIR_LIB})

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release")
endif()

message("Building: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)

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
  )

  add_compile_options(/W3 /bigobj)
else()
  add_compile_options(-Wall -Wextra -Wpedantic )
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

project(npsystem)

add_subdirectory(nplib)
add_subdirectory(nprpc)
add_subdirectory(npc)
add_subdirectory(npdbclient)
add_subdirectory(npdbserver)
add_subdirectory(npdbstatic)
add_subdirectory(npsys)
add_subdirectory(avr_info)
add_subdirectory(sim)
add_subdirectory(npserver)
add_subdirectory(npwebserver)
if(WIN32)
  add_subdirectory(npsystem)
endif()

