
add_library(npsys STATIC
  src/alg.cpp
  src/corba.cpp
  src/online_value.cpp
  src/system.cpp
)

target_include_directories(npsys PRIVATE 
  ../npsys/include
  ../npdbclient/include
  ../npc/include
  ../nplib/include
  ../nprpc/include
  ${npc_INCLUDE_DIR}  # For generated headers
)

if(WIN32)
  target_compile_definitions(npsys PRIVATE -DNPSYS_EXPORTS)
else()
  target_compile_options(npsys PRIVATE -fPIC)
endif()