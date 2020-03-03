cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}})
endif()

project(PrimalEngine 1.0 DESCRIPTION "C++ Game Engine" LANGUAGES CXX)

include(cmake/BasicSettings.cmake)

# Project Options as an interface library.
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Project Compile flags as an interface library.
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# Doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# Setup static analysis
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if (ENABLE_PCH)
  target_precompile_headers(project_options INTERFACE <vector> <string> <map> <utility>)
endif()

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests")
  add_subdirectory(tests)
endif()

add_subdirectory(src)
