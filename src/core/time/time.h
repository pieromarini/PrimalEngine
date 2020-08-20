#ifndef TIME_H
#define TIME_H

#include "engine_loop.h"

namespace primal {
  class Time {
	public:
	  static double getElapsedTime() {
		return EngineLoop::getGameClock().getElapsedTime();
	  }

	  static double getDeltaTime() {
		return EngineLoop::getGameClock().getDeltaTime();
	  }

	  static double getFixedDeltaTime() {
		static double fixedDeltaTime = 1.0 / 60.0f;
		return fixedDeltaTime;
	  }

	  static double getElapsedUnscaledTime() {
		return EngineLoop::getGameClock().getElapsedUnscaledTime();
	  }

	  static uint64_t getTimestamp() {
		return EngineLoop::getGameClock().getTimestamp();
	  }

	  static void setGameScale(const float newTimeScale) {
		EngineLoop::getGameClock().m_timeScale = newTimeScale;
	  }

	  static float getTimeScale() {
		return EngineLoop::getGameClock().m_timeScale;
	  }

	  static void pause() {
		EngineLoop::getGameClock().m_isPause = true;
	  }

	  static void resume() {
		EngineLoop::getGameClock().m_isPause = false;
	  }
  };

}

#endif
