cmake_policy(SET CMP0135 NEW)

# GoogleTest requires at least C++14

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(test_compiler
  src/test_compiler.cc
)

target_link_libraries(test_compiler
  GTest::gtest_main
  nprpcst
	npdbstatic
	npdbclient
	nplib
	npsys
	nprpc
	npc
  avr_info
  npcompiler
)

add_custom_command(TARGET test_compiler PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:test_compiler> $<TARGET_RUNTIME_DLLS:test_compiler>
  COMMAND_EXPAND_LISTS
)

target_include_directories(test_compiler PRIVATE
  ../avr_info/include
  ../npc/include
  ../nplib/include
  ../npdbclient/include
  ../npsys/include
  ../nprpc/include
  ../npcompiler/include
  ../avr_firmware/include
)

include(GoogleTest)
gtest_discover_tests(test_compiler)

set_target_properties(test_compiler PROPERTIES FOLDER "Tests")
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "Tests/GTest")

