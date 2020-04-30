cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}})
endif()

project(PrimalEngine DESCRIPTION "C++ Game Engine" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

include(cmake/BasicSettings.cmake)

# Project Options as an interface library.
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Project Compile flags as an interface library.
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# Sanitizer
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# Doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# Check that all submodules are present
include(cmake/SubmodulesCheck.cmake)
check_submodules()

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" ON)
option(ENABLE_TESTING "Enable Test Builds" OFF)

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if (ENABLE_PCH)
  target_precompile_headers(project_options INTERFACE <vector> <string> <map> <utility>)
endif()

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests")
  add_subdirectory(tests)
endif()


# ============ Libraries / Linking ============ #
include(cmake/SrcList.cmake)

set(LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern")
set(SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SANDBOX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sandbox")
set(ENGINE_LIB "primal_engine")
if (BUILD_SHARED_LIBS)
	add_library(${ENGINE_LIB} SHARED "${engine_sources}")
	set_target_properties(${ENGINE_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
else()
	add_library(${ENGINE_LIB} "${engine_sources}")
endif()

# Setup static analysis
include(cmake/StaticAnalyzers.cmake)

target_include_directories(${ENGINE_LIB} PRIVATE "${SOURCES_DIR}")

# [LIB] GLFW
set(GLFW_DIR "${LIB_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_include_directories(${ENGINE_LIB} PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(${ENGINE_LIB} PRIVATE "GLFW_INCLUDE_NONE")

# [LIB] glm
set(GLM_DIR "${LIB_DIR}/glm")
add_subdirectory("${GLM_DIR}")
target_include_directories(${ENGINE_LIB} PRIVATE "${GLM_DIR}")

# [LIB] GLAD
set(GLAD_DIR "${LIB_DIR}/glad")
add_library(glad "${GLAD_DIR}/src/glad.c")
target_include_directories(glad PRIVATE "${GLAD_DIR}/include")
target_include_directories(${ENGINE_LIB} PRIVATE "${GLAD_DIR}/include")

# [LIB] ImGUI
set(IMGUI_DIR "${LIB_DIR}/imgui")
add_library(imgui STATIC
	${IMGUI_DIR}/imconfig.h
	${IMGUI_DIR}/imgui.h
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_demo.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/examples/imgui_impl_glfw.h
	${IMGUI_DIR}/examples/imgui_impl_glfw.cpp
	${IMGUI_DIR}/examples/imgui_impl_opengl3.h
	${IMGUI_DIR}/examples/imgui_impl_opengl3.cpp
	${IMGUI_DIR}/imgui_internal.h
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/imstb_rectpack.h
	${IMGUI_DIR}/imstb_textedit.h
	${IMGUI_DIR}/imstb_truetype.h
)
if (BUILD_SHARED_LIBS)
	# NOTE: Need to enable PIC so we can build the shared library correctly.
	set_target_properties(imgui PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_include_directories(imgui PRIVATE "${IMGUI_DIR}" "${GLFW_DIR}/include")
target_include_directories(${ENGINE_LIB} PRIVATE "${IMGUI_DIR}")

# [LIB] SOIL
set(SOIL_DIR "${LIB_DIR}/SOIL")
add_subdirectory("${SOIL_DIR}")
target_include_directories(${ENGINE_LIB} PRIVATE "${SOIL_DIR}/include")

# [LIB] ASSIMP
set(ASSIMP_DIR "${LIB_DIR}/assimp")
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS  OFF)
set(ASSIMP_NO_EXPORT OFF)
set(ASSIMP_INSTALL OFF)
add_subdirectory("${ASSIMP_DIR}")
target_include_directories(${ENGINE_LIB} PRIVATE "${ASSIMP_DIR}/include")

# [LIB] spdlog
set(SPDLOG_DIR "${LIB_DIR}/spdlog")
add_subdirectory("${SPDLOG_DIR}")
if (BUILD_SHARED_LIBS)
	set(SPDLOG_BUILD_SHARED ON)
	set(SPDLOG_BUILD_TEXTS OFF)
	set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
target_include_directories(${ENGINE_LIB} PRIVATE "${SPDLOG_DIR}/include")

# Engine Library Linking
target_link_libraries(${ENGINE_LIB} glfw "${GLFW_LIBRARIES}" soil glad "${CMAKE_DL_LIBS}" imgui assimp spdlog::spdlog)

# [EXEC] Main Executable Linking
set(EXEC_NAME "sandbox")
add_executable(${EXEC_NAME} ${sandbox_sources})
target_include_directories(${EXEC_NAME} PRIVATE ${SANDBOX_DIR} ${GLM_DIR} ${SOURCES_DIR})
target_link_libraries(${EXEC_NAME} PRIVATE ${ENGINE_LIB} project_options project_warnings)

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_SOURCE_DIR}/res ${PROJECT_BINARY_DIR}/res RESULT_VARIABLE exitcode)

# NOTE: We move the compile_commands.json file to the project root so that completion engines can pick it up automatically.
# TODO: FIX. this should be the final step after running 'make'
if(EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
	file(RENAME "${CMAKE_BINARY_DIR}/compile_commands.json" "${PROJECT_SOURCE_DIR}/compile_commands.json")
endif()
