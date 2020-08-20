#include "application.h"
#include "engine_loop.h"

#include <stdexcept>
#include <string>

namespace primal {

  void Application::start() {

	if (EngineLoop::instance().m_isRunning) {
	  throw std::runtime_error("Engine already started.");
	}
	EngineLoop::instance().run();
  }

  void Application::exit() {
	EngineLoop::instance().m_isRunning = false;
  }

}
