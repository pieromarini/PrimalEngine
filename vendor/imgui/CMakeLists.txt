cmake_minimum_required(VERSION 3.0)
project(imgui)

set(IMGUI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/vendor/imgui)

file(GLOB IMGUI_SOURCES ${IMGUI_INCLUDE_DIR}/*.cpp)
file(GLOB IMGUI_HEADERS ${IMGUI_INCLUDE_DIR}/*.h)
                 
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

include_directories(
    ${IMGUI_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/vendor/glad/include)
    
target_link_libraries(imgui
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES})
    
set_target_properties(imgui PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(imgui PROPERTIES FOLDER vendor)

set(IMGUI_LIBRARIES imgui)
