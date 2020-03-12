if(UNIX)
	message(STATUS "Collecting source files.")
	execute_process(
		COMMAND bash -c "find ${PROJECT_SOURCE_DIR}/src -type f"
		COMMAND bash -c "tr '\n' ';'"
		OUTPUT_VARIABLE src_files
	)
	execute_process(
		COMMAND bash -c "find ${PROJECT_SOURCE_DIR}/sandbox -type f"
		COMMAND bash -c "tr '\n' ';'"
		OUTPUT_VARIABLE sandbox_files
	)
elseif(WIN32)
	message(FATAL_ERROR "Source file gathering is not implemented for Windows.")
else()
	message(FATAL_ERROR "Unknown platform.")
endif()


set(engine_sources ${src_files})

set(sandbox_sources ${sandbox_files})
