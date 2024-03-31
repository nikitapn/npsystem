add_executable(npserver
  src/bridge_avr_virtual.cpp
  src/stdafx.cpp
  src/npsys.cpp
  src/environment.cpp
  src/boost_export.cpp
  src/history.cpp
  src/avr_virtual.cpp
  src/bridge_udp.cpp
  src/main.cpp
  src/frame.cpp
  src/itemmgr.cpp
  src/protocol.cpp
  src/listener.cpp
)

add_definitions(
  -DBOOST_LOG_DYN_LINK
  -DNPSYSTEM_FIRMWARE_API_CPP
  -D_NPSERVER_
)

target_include_directories(npserver PRIVATE
  ../npc/include
	../avr_info/include
	../sim/include
	../nplib/include
	../npdbclient/include
	../npsys/include
	../avr_firmware/include
	../nprpc/include
)

target_link_libraries(npserver PUBLIC 
  nprpcst
	leveldb
	npdbstatic
	npdbclient
	sim
	nplib
	npsys
	avr_info
	nprpc
	npc
)

if(UNIX)
	target_link_libraries(npserver PUBLIC 
		/usr/lib/libboost_system.a
		/usr/lib/libboost_thread.a
		/usr/lib/libboost_serialization.a
		/usr/lib/libboost_log.a
		pthread
	)
endif()
