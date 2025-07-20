include(GoogleTest)
add_executable(test_nplib src/main.cpp)

target_include_directories(test_nplib PRIVATE
  ${CMAKE_SOURCE_DIR}/nplib/include
)

target_link_libraries(test_nplib PRIVATE
  gtest_main
)

enable_testing()
gtest_add_tests(TARGET test_nplib
  TEST_LIST Test
)
set_tests_properties(${Test} PROPERTIES TIMEOUT 10)
