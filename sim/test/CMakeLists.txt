include(GoogleTest)
add_executable(test_sim src/main.cpp)

target_include_directories(test_sim PRIVATE
  ${CMAKE_SOURCE_DIR}/avr_firmware/include
  ${CMAKE_SOURCE_DIR}/avr_info/include
  ${CMAKE_SOURCE_DIR}/sim/include
  ${CMAKE_SOURCE_DIR}/nplib/include
  ${CMAKE_SOURCE_DIR}/npdbclient/include
)

target_link_libraries(test_sim PRIVATE
    #gtest_main
    #gmock_main
    #gtest
    #gmock
    sim
    gtest_main
)

if(UNIX)
  target_link_libraries(test_sim PUBLIC
    ${BOOST_LIB_PREFIX}/libboost_system.a
    ${BOOST_LIB_PREFIX}/libboost_thread.a
    ${BOOST_LIB_PREFIX}/libboost_serialization.a
    ${BOOST_LIB_PREFIX}/libboost_log.a
    pthread
  )
endif()

enable_testing()
gtest_add_tests(TARGET test_sim
  TEST_LIST Test
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/firmware/out)
set_tests_properties(${Test} PROPERTIES TIMEOUT 10)
