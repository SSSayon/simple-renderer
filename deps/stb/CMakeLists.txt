cmake_minimum_required(VERSION 3.10)
project(stb)

add_library(stb STATIC
    stb_image.cpp
)

target_include_directories(stb PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)