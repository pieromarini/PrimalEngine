cmake_minimum_required(VERSION 3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.27)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}})
endif()

project(
  PrimalEngine
  DESCRIPTION "C++ Game Engine"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Intel Threading Library.
find_package(TBB COMPONENTS tbbmalloc tbbmalloc_proxy tbb_preview)

include(cmake/BasicSettings.cmake)

# Project Options as an interface library.
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)

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

# FetchContent config
include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${PROJECT_SOURCE_DIR}/extern CACHE PATH "Missing description." FORCE)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" ON)
option(ENABLE_TESTING "Enable Test Builds" OFF)

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if(ENABLE_PCH)
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
if(BUILD_SHARED_LIBS)
  add_library(${ENGINE_LIB} SHARED "${engine_sources}")
  set_target_properties(${ENGINE_LIB} PROPERTIES POSITION_INDEPENDENT_CODE ON)
else()
  add_library(${ENGINE_LIB} "${engine_sources}")
endif()

# Setup static analysis
include(cmake/StaticAnalyzers.cmake)

target_include_directories(${ENGINE_LIB} PRIVATE "${SOURCES_DIR}")

target_compile_definitions(engine PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE)

# [LIB] GLFW
set(GLFW_DIR "${LIB_DIR}/glfw")
add_subdirectory("${GLFW_DIR}")
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
target_include_directories(${ENGINE_LIB} PUBLIC "${GLFW_DIR}/include")
target_compile_definitions(${ENGINE_LIB} PUBLIC "GLFW_INCLUDE_NONE")

# [LIB] Vulkan
find_package(Vulkan REQUIRED)
target_include_directories(${ENGINE_LIB} PUBLIC "${Vulkan_INCLUDE_DIRS}")
# TODO: Vulkan_GLSLC_EXECUTABLE -> GLSL SPIR-V compiler
# Vulkan_GLSLANG_VALIDATOR_EXECUTABLE -> glslangValidator

# [LIB] STB_IMAGE
set(STBIMAGE_DIR "${LIB_DIR}/stb_image")
target_include_directories(${ENGINE_LIB} PUBLIC "${STBIMAGE_DIR}")

# [LIB] ASSIMP
FetchContent_Declare(assimp GIT_REPOSITORY https://github.com/assimp/assimp.git GIT_TAG master)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(assimp)
target_include_directories(${ENGINE_LIB} PRIVATE extern/assimp-src/include)

# [LIB] SDL3
FetchContent_Declare(sdl3 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG main)
set(BUILD_EXAMPLES OFF CACHE INTERNAL "" FORCE)
FetchContent_MakeAvailable(sdl3)
target_include_directories(${ENGINE_LIB} PRIVATE extern/sdl3-src/include)

# Engine Library Linking
target_link_libraries(${ENGINE_LIB} PRIVATE
  "${Vulkan_LIBRARY}"
  "${CMAKE_DL_LIBS}"
  SDL3::SDL3
  assimp
  tbb
)

find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin)

foreach(GLSL ${glsl_sources})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
  Shaders 
  DEPENDS ${SPIRV_BINARY_FILES}
)

# [EXEC] Main Executable Linking
set(EXEC_NAME "sandbox")
add_executable(${EXEC_NAME} ${sandbox_sources})
target_include_directories(${EXEC_NAME} PUBLIC ${SANDBOX_DIR} ${SOURCES_DIR})
target_link_libraries(${EXEC_NAME} PUBLIC ${ENGINE_LIB} project_options project_warnings)

execute_process(
  COMMAND ${CMAKE_COMMAND} -E create_symlink ${PROJECT_SOURCE_DIR}/res
          ${PROJECT_BINARY_DIR}/res RESULT_VARIABLE exitcode)
