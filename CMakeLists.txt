cmake_minimum_required(VERSION 3.28.1)

#set(OUTPUT_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
#link_directories(${OUTPUT_DIR_LIB})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    add_compile_options(-std=c++20 -Wall -Wextra -Wpedantic )
endif()

project(npsystem)

add_subdirectory(nplib)
add_subdirectory(nprpc)
add_subdirectory(npc)
add_subdirectory(npdbclient)
add_subdirectory(npdbserver)
add_subdirectory(npdbstatic)
add_subdirectory(npsys)
add_subdirectory(avr_info)
add_subdirectory(sim)
add_subdirectory(npserver)
add_subdirectory(npwebserver)

