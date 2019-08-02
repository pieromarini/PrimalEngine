cmake_minimum_required(VERSION 3.0)
project(Glad)

set(GLAD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/vendor/glad/include)

add_library(glad STATIC include/glad/gl.h src/gl.c)

set(GLAD_LIBRARIES glad)
target_include_directories(glad PUBLIC include/)
