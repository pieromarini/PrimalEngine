#ifndef LOG_H
#define LOG_H

#include "core/core.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace primal {

  class PRIMAL_API Log {
	public:
	  static void init();

	  inline static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
	  inline static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
	private:
	  static std::shared_ptr<spdlog::logger> s_CoreLogger;
	  static std::shared_ptr<spdlog::logger> s_ClientLogger;
  };

}

// Core log macros

template<typename ...Args>
constexpr void PRIMAL_CORE_TRACE(Args ...args) {
  primal::Log::getCoreLogger()->trace(args...);
}
template<typename ...Args>
constexpr void PRIMAL_CORE_INFO(Args ...args) {
  primal::Log::getCoreLogger()->info(args...);
}
template<typename ...Args>
constexpr void PRIMAL_CORE_WARN(Args ...args) {
  primal::Log::getCoreLogger()->warn(args...);
}
template<typename ...Args>
constexpr void PRIMAL_CORE_ERROR(Args ...args) {
  primal::Log::getCoreLogger()->error(args...);
}
template<typename ...Args>
constexpr void PRIMAL_CORE_CRITICAL(Args ...args) {
  primal::Log::getCoreLogger()->critical(args...);
}

template<typename ...Args>
constexpr void PRIMAL_TRACE(Args ...args) {
  primal::Log::getClientLogger()->trace(args...);
}
template<typename ...Args>
constexpr void PRIMAL_INFO(Args ...args) {
  primal::Log::getClientLogger()->info(args...);
}
template<typename ...Args>
constexpr void PRIMAL_WARN(Args ...args) {
  primal::Log::getClientLogger()->warn(args...);
}
template<typename ...Args>
constexpr void PRIMAL_ERROR(Args ...args) {
  primal::Log::getClientLogger()->error(args...);
}
template<typename ...Args>
constexpr void PRIMAL_CRITICAL(Args ...args) {
  primal::Log::getClientLogger()->critical(args...);
}

#endif
