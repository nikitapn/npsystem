list(APPEND IDL_FILES_LIST 
  idl/db.npidl 
  idl/npsystem_constants.npidl
  idl/server.npidl
  idl/npwebserver.npidl
)

foreach(file ${IDL_FILES_LIST})
  string(REPLACE "/" "_" tmp ${file})
  string(REPLACE "idl/" "" filename ${file})
  string(REPLACE ".npidl" ".cpp" filename_cpp ${filename})
  string(REPLACE ".npidl" ".hpp" filename_hpp ${filename})
  string(REPLACE ".npidl" ".ts" filename_ts ${filename})
  add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/npc/include/npc/${filename_hpp} 
      ${CMAKE_SOURCE_DIR}/npc/src/gen/${filename_cpp}
      COMMAND npidl
      --out-inc-dir ${CMAKE_SOURCE_DIR}/npc/include/npc 
      --out-src-dir ${CMAKE_SOURCE_DIR}/npc/src/gen
      --out-ts-dir ${CMAKE_SOURCE_DIR}/nprpc/idl/npwebserver/web/src/gen
      ${CMAKE_SOURCE_DIR}/npc/idl/${filename}
    DEPENDS ${file}
    BYPRODUCTS ${CMAKE_SOURCE_DIR}/npc/include/npc/${filename_hpp} 
       ${CMAKE_SOURCE_DIR}/npc/src/gen/${filename_cpp}
       ${CMAKE_SOURCE_DIR}/nprpc/idl/npwebserver/web/src/gen/${filename_ts}
  )
endforeach()

add_library(npc STATIC
  include/npc/db.hpp
  include/npc/npsystem_constants.hpp
  include/npc/npwebserver.hpp
  include/npc/server.hpp
  src/gen/db.cpp
  src/gen/npwebserver.cpp
  src/gen/server.cpp
)

target_include_directories(npc PRIVATE ./include/npc ../nprpc/include)

if(UNIX)
  target_compile_options(npc PRIVATE -fPIC)
endif()