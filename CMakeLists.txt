cmake_minimum_required(VERSION 3.28.1)

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



