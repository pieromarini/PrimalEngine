cmake_minimum_required(VERSION 3.1...3.15)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}})
endif()

project(PrimalEngine 1.0 DESCRIPTION "C++ Game Engine" LANGUAGES CXX)


# Setup clang-tidy
if(CMAKE_VERSION VERSION_GREATER 3.6)
    option(CLANG_TIDY_FIX "Perform fixes for Clang-Tidy" OFF)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy" DOC "Path to clang-tidy executable")

    if(CLANG_TIDY_EXE)
        if(CLANG_TIDY_FIX)
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}" "-fix")
        else()
            set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
        endif()
    endif()
endif()



# TODO: Create target first

# target_compile_features(primal_engine PUBLIC cxx_std_17)
# set_target_properties(primal_engine PROPERTIES CXX_EXTENSIONS OFF)
