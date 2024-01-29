list(APPEND SOURCES src/db.cpp)
if (MSVC)
  list(APPEND SOURCES src/dllmain.cpp)
endif()

add_library(npdbclient SHARED ${SOURCES})

target_include_directories(npdbclient PRIVATE 
  ./include 
  ../nprpc/include 
  ../nplib/include 
  ../npc/include
)

target_compile_options(nplib PRIVATE -fPIC)

