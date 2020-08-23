#ifndef ENGINELOOP_H
#define ENGINELOOP_H

#include "application.h"
#include "core/time/clock.h"

namespace primal {

  class EngineLoop {
	public:
	  struct LoopConfig {
		int maxFps { 60 };
	  };

	  EngineLoop();
	  ~EngineLoop();

	  static EngineLoop& instance();

	  static Clock& getGameClock();

	private:
	  bool m_isRunning;

	  class GraphicsModule* m_graphicsModule;
	  class WindowModule* m_windowModule;
	  class InputModule* m_inputModule;

	  void init();

	  void run();

	  void update();
	  void fixedUpdate(const float deltaTime) const;
	  void variableUpdate(const float deltaTime) const;

	  void shutdown();

	  void startGameClock() const;

	  friend class Application;
  };

}

#endif
