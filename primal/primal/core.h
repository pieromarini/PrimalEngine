#pragma once

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

// Assert statements
// PRIMAL_ASSERT      will assert the passed statement; if not PRIMAL_ENABLE_ASSERTS will do nothing
// PRIMAL_ASSERT_CALL will assert the passed statement; if not PRIMAL_ENABLE_ASSERTS will only call the statement
#if defined PRIMAL_ENABLE_ASSERTS
	#define PRIMAL_ASSERT(x, ...) { if(!(x)) { PRIMAL_ERROR("Assertion Failed: {0}", __VA_ARGS__); PRIMAL_DEBUGBREAK(); } }
	#define PRIMAL_CORE_ASSERT(x, ...) { if(!(x)) { PRIMAL_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); PRIMAL_DEBUGBREAK(); } }

	#define PRIMAL_ASSERT_CALL(x, ...) PRIMAL_ASSERT(x, __VA_ARGS__)
	#define PRIMAL_CORE_ASSERT_CALL(x, ...) PRIMAL_CORE_ASSERT(x, __VA_ARGS__)
#else
	#define PRIMAL_ASSERT(x, ...)
	#define PRIMAL_CORE_ASSERT(x, ...)

	#define PRIMAL_ASSERT_CALL(x, ...) x
	#define PRIMAL_CORE_ASSERT_CALL(x, ...) x
#endif

#define BIT(x) (1 << x)

#define PRIMAL_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
