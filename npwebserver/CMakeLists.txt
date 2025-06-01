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
	npdbstatic
	npdbclient
	nplib
	npsys
	nprpc
	npc
)

if(UNIX)
  target_link_libraries(npwebserver PUBLIC 
    ${BOOST_LIB_PREFIX}/libboost_system.a
    ${BOOST_LIB_PREFIX}/libboost_thread.a
    ${BOOST_LIB_PREFIX}/libboost_serialization.a
    pthread
  )
endif()

