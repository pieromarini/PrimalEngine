cmake_minimum_required(VERSION 3.1)
project(primal)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall")

set(PRIMAL_CORE_LIB "primal_core")
set(SANDBOX_APP "sandbox")

set(VENDOR_LIB "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/primal")
set(SANDBOX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sandbox")

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

if(NOT TARGET spdlog)
	find_package(spdlog REQUIRED)
endif()

include_directories(
	${SRC_DIR}
	${SANDBOX_DIR}
	${VENDOR_LIB}/glfw/include
	${VENDOR_LIB}/imgui
	${VENDOR_LIB}/glad/include
	${VENDOR_LIB}/spdlog
)


add_library(${PRIMAL_CORE_LIB} STATIC ${CORE_SRCS})
# add_custom_target(primal_static ALL COMMAND ${CMAKE_AR} rc primal_static.a 
#	<TARGET_FILE:${PRIMAL_CORE_LIB}>
# )

add_executable(${SANDBOX_APP} ${SANDBOX_SRCS})
target_link_libraries(${SANDBOX_APP} ${PRIMAL_CORE_LIB} glfw ${GLFW_LIBRARIES} glad imgui ${IMGUI_LIBRARIES} spdlog::spdlog)
