add_executable(make_info
  src/atmega16.cpp
  src/atmega16v.cpp
  src/atmega8.cpp
  src/atmega8v.cpp
  src/avr.h
  src/iom16.h
  src/iom8.h
  src/make_info.cpp
  src/pclink.cpp
  src/pclink_virtual.cpp
)

target_include_directories(make_info PRIVATE
  ../avr_info/include
  ../nplib/include
  ../npsys/include
)

if(UNIX)
  target_link_libraries(make_info PUBLIC 
    /usr/lib/libboost_system.a
    /usr/lib/libboost_serialization.a
    boost_program_options
  )
endif()
