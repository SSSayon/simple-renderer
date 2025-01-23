cmake_minimum_required(VERSION 3.10)
project(tinyobjloader)

add_library(tinyobjloader STATIC
    tiny_obj_loader.cc
)

target_include_directories(tinyobjloader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)