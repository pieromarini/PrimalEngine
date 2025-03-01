if(UNIX)
	message(STATUS "Collecting UNIX source files.")
	execute_process(
		COMMAND bash -c "find ${PROJECT_SOURCE_DIR}/src -type f"
		COMMAND bash -c "tr '\n' ';'"
		OUTPUT_VARIABLE src_files
	)
	file(GLOB_RECURSE glsl_sources
		"${PROJECT_SOURCE_DIR}/res/shaders/*.frag"
		"${PROJECT_SOURCE_DIR}/res/shaders/*.vert"
		"${PROJECT_SOURCE_DIR}/res/shaders/*.comp"
	)

	file(GLOB_RECURSE glsl_header_files 
		"${PROJECT_SOURCE_DIR}/res/shaders/*.h" 
		"${PROJECT_SOURCE_DIR}/res/shaders/config.h"
	)

	file(GLOB_RECURSE sandbox_files
		"${PROJECT_SOURCE_DIR}/sandbox/*.h"
		"${PROJECT_SOURCE_DIR}/sandbox/*.hpp"
		"${PROJECT_SOURCE_DIR}/sandbox/*.c"
		"${PROJECT_SOURCE_DIR}/sandbox/*.cpp"
	)
elseif(WIN32)
	message(STATUS "Collecting Windows source files.")
	file(GLOB_RECURSE src_files
		"${PROJECT_SOURCE_DIR}/src/*.h"
		"${PROJECT_SOURCE_DIR}/src/*.hpp"
		"${PROJECT_SOURCE_DIR}/src/*.c"
		"${PROJECT_SOURCE_DIR}/src/*.cpp"
	)
	file(GLOB_RECURSE glsl_sources
		"${PROJECT_SOURCE_DIR}/res/shaders/*.frag"
		"${PROJECT_SOURCE_DIR}/res/shaders/*.vert"
		"${PROJECT_SOURCE_DIR}/res/shaders/*.comp"
	)
	file(GLOB_RECURSE glsl_header_files 
		"${PROJECT_SOURCE_DIR}/res/shaders/*.h" 
		"${PROJECT_SOURCE_DIR}/res/shaders/config.h"
	)
	file(GLOB_RECURSE sandbox_files
		"${PROJECT_SOURCE_DIR}/sandbox/*.h"
		"${PROJECT_SOURCE_DIR}/sandbox/*.hpp"
		"${PROJECT_SOURCE_DIR}/sandbox/*.c"
		"${PROJECT_SOURCE_DIR}/sandbox/*.cpp"
	)
else()
	message(FATAL_ERROR "Unknown platform.")
endif()


set(engine_sources ${src_files})

set(sandbox_sources ${sandbox_files})
