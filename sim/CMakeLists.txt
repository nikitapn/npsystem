add_library(sim SHARED
  src/boost_export.cpp
  src/cpu.cpp
  src/hexparser.cpp
  src/main.cpp
  src/medium.cpp
  src/stdafx.cpp
  src/vm.cpp
)

add_definitions(
  -DBOOST_LOG_DYN_LINK
  -DNPSYSTEM_FIRMWARE_API_CPP
)

target_include_directories(sim PRIVATE 
  ../sim/include
  ../nplib/include
  ../npdb/include
  ../npdbclient/include
	../avr_firmware/include
)

target_link_libraries(sim PUBLIC 
  npdbstatic
  npdbclient
  nplib
)

if(WIN32)
  target_compile_definitions(sim PRIVATE -DSIM_EXPORTS)
else()
  target_link_libraries(sim PUBLIC 
    /usr/lib/libboost_system.a
    pthread
  )
  target_compile_options(sim PRIVATE -fPIC)
endif()