
add_library(npsys STATIC
  src/npsys.cpp

  include/npsys/access.h
  include/npsys/algcat.h
  include/npsys/algsubcat.h
  include/npsys/avr_controller.h
  include/npsys/boost_export.hpp
  include/npsys/cmodels.h
  include/npsys/control_unit.h
  include/npsys/common_rpc.hpp
  include/npsys/fat_data_callback.h
  include/npsys/fbdblock.h
  include/npsys/fundamental.h
  include/npsys/guid.h
  include/npsys/header.h
  include/npsys/memtypes.h
  include/npsys/network.h
  include/npsys/other
  include/npsys/strings.h
  include/npsys/system.h
  include/npsys/variable.h
  include/npsys/variable.inl
  include/npsys/web.h

  include/npsys/other/online_value.h
  include/npsys/other/remote.h
  include/npsys/other/uploadable.h
  include/npsys/other/write_default_value_request.h

  include/npsys/cpp/cpp_slot.h
  include/npsys/cpp/cpp_variable.h
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
