add_library(npdbstatic STATIC
  src/binary_iarchive_special.cpp
  src/binary_oarchive_special.cpp
)

target_include_directories(npdbstatic PRIVATE ../npdbclient/include)

if(UNIX)
  target_compile_options(npdbstatic PRIVATE -fPIC)
endif()
