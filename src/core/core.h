#ifndef CORE_H
#define CORE_H

// Temporary for local testing.
// NOTE: Will be defined inside the CMake build system with add_compile_definitions
#define PRIMAL_PLATFORM_LINUX
#define PRIMAL_DYNAMIC_LINK
#define PRIMAL_BUILD_DLL
#define PRIMAL_DEBUG


// Dynamic Link Library
#if defined PRIMAL_PLATFORM_WINDOWS
	#if defined PRIMAL_DYNAMIC_LINK
		#if defined PRIMAL_BUILD_DLL
			#define PRIMAL_API __declspec(dllexport)
		#else
			#define PRIMAL_API __declspec(dllimport)
		#endif
	#else
		#define PRIMAL_API
	#endif
#elif defined PRIMAL_PLATFORM_LINUX
	#if defined PRIMAL_DYNAMIC_LINK
		#if defined PRIMAL_BUILD_DLL
			#define PRIMAL_API __attribute__((visibility("default")))
		#else
			#define PRIMAL_API
		#endif
	#else
		#define PRIMAL_API
	#endif

#else
	#error Unsupported platform!
#endif

// Debug settings
#if defined PRIMAL_DEBUG
	#if defined PRIMAL_PLATFORM_WINDOWS
		#define PRIMAL_DEBUGBREAK() __debugbreak()
	#elif defined PRIMAL_PLATFORM_LINUX
		#include <csignal>
		#define PRIMAL_DEBUGBREAK() raise(SIGTRAP)
	#endif

	#define PRIMAL_ENABLE_ASSERTS
#endif // End of Debug settings

// TODO: move to Asserts Module inside "Core"
#ifdef PRIMAL_ENABLE_ASSERTS
	#define PRIMAL_ASSERT(x, ...) { if(!(x)) { PRIMAL_ERROR("Assertion Failed: {0}", __VA_ARGS__); PRIMAL_DEBUGBREAK(); } }
	#define PRIMAL_CORE_ASSERT(x, ...) { if(!(x)) { PRIMAL_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); PRIMAL_DEBUGBREAK(); } }
#else
	#define PRIMAL_ASSERT(x, ...)
	#define PRIMAL_CORE_ASSERT(x, ...)
#endif

#endif
