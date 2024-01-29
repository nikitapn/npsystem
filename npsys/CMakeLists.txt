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
)

target_compile_options(nplib PRIVATE -fPIC)
