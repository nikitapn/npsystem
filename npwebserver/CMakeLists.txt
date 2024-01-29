add_executable(npwebserver
  src/npsys.cpp
  src/boost_export.cpp
  src/main.cpp
)

add_definitions(
  -DNPSYSTEM_FIRMWARE_API_CPP
  -D_NPWEBSERVER_
)

target_include_directories(npwebserver PRIVATE
  ../avr_info/include
  ../npc/include
  ../nplib/include
  ../npdbclient/include
  ../npsys/include
  ../nprpc/include
)

target_link_libraries(npwebserver PUBLIC 
  /usr/lib/libboost_system.a
	/usr/lib/libboost_thread.a
	/usr/lib/libboost_serialization.a
  nprpcst
  pthread
	npdbstatic
	npdbclient
	nplib
	npsys
	nprpc
	npc
)

