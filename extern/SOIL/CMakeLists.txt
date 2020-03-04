cmake_minimum_required (VERSION 3.2)
set (SOIL_TARGET soil)
project (${SOIL_TARGET})

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(INCLUDE_FILES
	"${INCLUDE_DIR}/SOIL/image_DXT.h"
	"${INCLUDE_DIR}/SOIL/image_helper.h"
	"${INCLUDE_DIR}/SOIL/SOIL.h"
	"${INCLUDE_DIR}/SOIL/stb_image_aug.h"
	"${INCLUDE_DIR}/SOIL/stbi_DDS_aug_c.h"
	"${INCLUDE_DIR}/SOIL/stbi_DDS_aug.h")

set (SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set (SRC_FILES
	"${SRC_DIR}/image_DXT.c"
	"${SRC_DIR}/image_helper.c"
	"${SRC_DIR}/SOIL.c"
	"${SRC_DIR}/stb_image_aug.c")

add_library(${SOIL_TARGET})

target_include_directories(${SOIL_TARGET}
	PUBLIC ${INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}
	PRIVATE "${INCLUDE_DIR}/SOIL")

target_sources(${SOIL_TARGET}
	PUBLIC ${INCLUDE_FILES}
	PRIVATE ${SRC_FILES})

set(OpenGL_GL_PREFERENCE "GLVND")

find_package(OpenGL REQUIRED)
target_link_libraries(${SOIL_TARGET} ${OPENGL_LIBRARIES})
