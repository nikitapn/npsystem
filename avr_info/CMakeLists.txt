add_library(avr_info STATIC 
  src/avr_info.cpp
  src/m8.cpp
  src/m16.cpp
  include/avr_info/avr_info.h
)

target_include_directories(avr_info PRIVATE 
  ../avr_info/include
	../nplib/include
	../npsys/include
)

if(UNIX)
  target_compile_options(avr_info PRIVATE -fPIC)
endif()
