cmake_minimum_required(VERSION 3.1)
project(primal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall")

set(PRIMAL_CORE_LIB "primal_core")
set(SANDBOX_APP "sandbox")

set(VENDOR_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/primal")
set(SANDBOX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sandbox")

set(OpenGL_GL_PREFERENCE "GLVND")

# Cache Vars
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

file(GLOB_RECURSE CORE_SRCS
	${SRC_DIR}/*.h
	${SRC_DIR}/*.cpp
)
file(GLOB_RECURSE SANDBOX_SRCS
	${SANDBOX_DIR}/*.h
	${SANDBOX_DIR}/*.cpp
)

add_subdirectory(${VENDOR_LIB}/glad)
add_subdirectory(${VENDOR_LIB}/glfw)
add_subdirectory(${VENDOR_LIB}/imgui)
add_subdirectory(${VENDOR_LIB}/soil)
add_subdirectory(${VENDOR_LIB}/assimp)


if(NOT TARGET spdlog)
	find_package(spdlog REQUIRED)
endif()

find_package(OpenGL REQUIRED)
find_library(SOIL SOIL vendor/soil/build/libsoil.a)

include_directories(
	${SRC_DIR}
	${SANDBOX_DIR}
	${VENDOR_LIB}/glfw/include
	${VENDOR_LIB}/imgui
	${VENDOR_LIB}/glad/include
	${VENDOR_LIB}/spdlog
	${VENDOR_LIB}/soil/inc
	${VENDOR_LIB}/assimp/include
)

set(ASSIMP_BUILD_STATIC_LIB ON)
set(ASSIMP_BUILD_TEST OFF)


add_library(${PRIMAL_CORE_LIB} STATIC ${CORE_SRCS})

add_executable(${SANDBOX_APP} ${SANDBOX_SRCS})
target_link_libraries(${SANDBOX_APP} ${PRIMAL_CORE_LIB} assimp glfw ${GLFW_LIBRARIES} ${OPENGL_LIBRARY} soil glad imgui ${IMGUI_LIBRARIES} spdlog::spdlog)
execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${primal_SOURCE_DIR}/res ${primal_BINARY_DIR}/res RESULT_VARIABLE exitcode)
