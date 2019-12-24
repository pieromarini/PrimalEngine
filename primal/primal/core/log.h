#pragma once

#include "core.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace primal {

  class Log {
	public:
	  static void init();

	  inline static ref_ptr<spdlog::logger>& getCoreLogger() { return s_coreLogger; }
	  inline static ref_ptr<spdlog::logger>& getClientLogger() { return s_clientLogger; }
	private:
	  static ref_ptr<spdlog::logger> s_coreLogger;
	  static ref_ptr<spdlog::logger> s_clientLogger;
  };

}

// Core log macros
#define PRIMAL_CORE_TRACE(...)    ::primal::Log::getCoreLogger()->trace(__VA_ARGS__)
#define PRIMAL_CORE_INFO(...)     ::primal::Log::getCoreLogger()->info(__VA_ARGS__)
#define PRIMAL_CORE_WARN(...)     ::primal::Log::getCoreLogger()->warn(__VA_ARGS__)
#define PRIMAL_CORE_ERROR(...)    ::primal::Log::getCoreLogger()->error(__VA_ARGS__)
#define PRIMAL_CORE_CRITICAL(...) ::primal::Log::getCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define PRIMAL_TRACE(...)         ::primal::Log::getClientLogger()->trace(__VA_ARGS__)
#define PRIMAL_INFO(...)          ::primal::Log::getClientLogger()->info(__VA_ARGS__)
#define PRIMAL_WARN(...)          ::primal::Log::getClientLogger()->warn(__VA_ARGS__)
#define PRIMAL_ERROR(...)         ::primal::Log::getClientLogger()->error(__VA_ARGS__)
#define PRIMAL_CRITICAL(...)      ::primal::Log::getClientLogger()->critical(__VA_ARGS__)
