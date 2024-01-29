add_executable(npdbserver
  src/npdbserver.cpp
  src/config.h
)

add_definitions(-DBOOST_LOG_DYN_LINK)

target_include_directories(npdbserver PRIVATE
  ../nplib/include
	../npdbclient/include
	../npc/include
  ../nprpc/include
)

target_link_libraries(npdbserver
  /usr/lib/libboost_system.a
	/usr/lib/libboost_serialization.a
  ${CMAKE_BINARY_DIR}/nprpc/libnprpcst.a
  pthread
	leveldb
	npc
	npdbstatic
	boost_log
	nprpc
	systemd
)
