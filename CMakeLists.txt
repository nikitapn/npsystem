cmake_minimum_required(VERSION 3.28.1)

set(BOOST_LIB_PREFIX "/usr/lib" CACHE STRING "Location of the boost libs")
#set_property(CACHE BOOST_LIB_PREFIX PROPERTY STRINGS "/usr/lib" "/usr/local/lib")

option(OPT_BUILD_ONLY_RPC "<help_text>" OFF)

#set(OUTPUT_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
#link_directories(${OUTPUT_DIR_LIB})

if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release")
endif()

message("Building: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

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
  )

  add_compile_options(/W3 /bigobj)
else()
  add_compile_options(-Wall -Wextra -Wpedantic )
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

project(npsystem)

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

  if(WIN32)
  #  add_subdirectory(npcompiler)
    add_subdirectory(npsystem)
  #  add_subdirectory(tests)

    add_subdirectory(avr_firmware)
    add_subdirectory(make_info)

    add_custom_command(
      OUTPUT generate_info.stamp
      COMMENT "Creating memory addresses file"
      COMMAND ${CMAKE_COMMAND} -E touch generate_info.stamp
      COMMAND make_info --out-dir $<TARGET_FILE_DIR:npsystem>/data
      DEPENDS avr_firmware make_info
    )

    add_custom_target(avr DEPENDS generate_info.stamp)

  endif()
endif()