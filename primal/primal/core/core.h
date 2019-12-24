#pragma once

#include <memory>

// Temporary for local testing.
// NOTE: Will be defined inside the CMake build system with add_compile_definitions
#define PRIMAL_PLATFORM_LINUX
#define PRIMAL_DYNAMIC_LINK
#define PRIMAL_BUILD_DLL
#define PRIMAL_DEBUG

// Debug settings
#ifdef PRIMAL_DEBUG
	#if defined PRIMAL_PLATFORM_WINDOWS
		#define PRIMAL_DEBUGBREAK() __debugbreak()
	#elif defined PRIMAL_PLATFORM_LINUX
		#include <csignal>
		#define PRIMAL_DEBUGBREAK() raise(SIGTRAP)
	#endif

	#define PRIMAL_ENABLE_ASSERTS
#endif

// Assert statements
#ifdef PRIMAL_ENABLE_ASSERTS
	#define PRIMAL_ASSERT(x, ...) { if(!(x)) { PRIMAL_ERROR("Assertion Failed: {0}", __VA_ARGS__); PRIMAL_DEBUGBREAK(); } }
	#define PRIMAL_CORE_ASSERT(x, ...) { if(!(x)) { PRIMAL_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); PRIMAL_DEBUGBREAK(); } }
#else
	#define PRIMAL_ASSERT(x, ...)
	#define PRIMAL_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define PRIMAL_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace primal {

  template<typename T>
  using scope_ptr = std::unique_ptr<T>;

  template<typename T, typename ... Args>
  constexpr scope_ptr<T> createScope(Args&& ... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
  }

  template<typename T>
  using ref_ptr = std::shared_ptr<T>;

  template<typename T, typename ... Args>
  constexpr ref_ptr<T> createRef(Args&& ... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
  }

}
