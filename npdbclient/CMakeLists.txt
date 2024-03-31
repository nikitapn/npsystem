list(APPEND SOURCES src/db.cpp)
if (WIN32)
  list(APPEND SOURCES src/dllmain.cpp)
endif()

add_library(npdbclient SHARED ${SOURCES})

target_include_directories(npdbclient PRIVATE 
  ./include 
  ../nprpc/include 
  ../nplib/include 
  ../npc/include
)

if(WIN32)
  target_compile_definitions(npdbclient PRIVATE -DNPDBCLIENT_EXPORTS)
else()
  target_compile_options(npdbclient PRIVATE -fPIC)
endif()

target_link_libraries(npdbclient PRIVATE
  npdbstatic
  npc
  nprpc
  nprpcst
)