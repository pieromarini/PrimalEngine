cmake_minimum_required(VERSION 2.6)
project(Soil)

set(SOIL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/vendor/soil/inc)

add_library(soil STATIC
  src/image_helper.c
  src/stb_image_aug.c
  src/image_DXT.c
  src/SOIL.c
)

set(SOIL_LIBRARIES soil)
target_include_directories(glad PUBLIC inc/)
