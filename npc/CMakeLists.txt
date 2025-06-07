include(${CMAKE_SOURCE_DIR}/nprpc/cmake/npidl.cmake)

set(IDL_FILES 
  ${CMAKE_SOURCE_DIR}/npc/idl/db.npidl 
  ${CMAKE_SOURCE_DIR}/npc/idl/npsystem_constants.npidl
  ${CMAKE_SOURCE_DIR}/npc/idl/server.npidl
  ${CMAKE_SOURCE_DIR}/npc/idl/npwebserver.npidl
)

npidl_generate_idl_files("${IDL_FILES}" npc)

add_library(npc STATIC
  ${npc_GENERATED_SOURCES}
  ${npc_GENERATED_HEADERS}
  # Add any additional non-generated source files here
)

# Make sure generated files are built before the library
add_dependencies(npc npc_gen)

target_include_directories(npc 
  PUBLIC 
    ${npc_INCLUDE_DIR}               # For generated headers
  PRIVATE 
    ../nprpc/include                 # For nprpc headers
)

if(UNIX)
  target_compile_options(npc PRIVATE -fPIC)
endif()

set(npc_INCLUDE_DIR ${npc_INCLUDE_DIR} PARENT_SCOPE)