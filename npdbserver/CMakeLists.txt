add_executable(npdbserver
  src/npdbserver.cpp
  src/config.h
)

target_include_directories(npdbserver PRIVATE
  ../nplib/include
	../npdbclient/include
	../npc/include
  ../nprpc/include
)

target_link_libraries(npdbserver
	leveldb
	npc
	npdbstatic
	nprpc
	nprpcst
	nplib
)

if(UNIX)
	add_definitions(-DBOOST_LOG_DYN_LINK)
	target_link_libraries(npdbserver
		/usr/lib/libboost_system.a
		/usr/lib/libboost_serialization.a
		boost_log
		pthread
		systemd
	)
endif()

