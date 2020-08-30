#include "clock.h"
#include <chrono>

namespace primal {

  Clock::Clock() : m_startTime{ HighResClock::now() },
				   m_currentTime{ HighResClock::now() }, 
				   m_deltaTime{},
				   m_elapsedTime{ 0 },
				   m_elapsedUnscaledTime{ 0 },
				   m_timeFrame{ 0 },
				   m_timeScale{ 1.0f },
				   m_isPause{ false } {}

  Clock::Clock(const Clock& clock)
				 : m_startTime{ clock.m_startTime },
				   m_currentTime{ clock.m_currentTime }, 
				   m_deltaTime{ clock.m_deltaTime },
				   m_elapsedTime{ clock.m_elapsedTime },
				   m_elapsedUnscaledTime{ clock.m_elapsedUnscaledTime },
				   m_timeFrame{ clock.m_timeFrame },
				   m_timeScale{ clock.m_timeScale },
				   m_isPause{ clock.m_isPause } {}

  Clock::Clock(Clock&& clock) noexcept
				 : m_startTime{ clock.m_startTime },
				   m_currentTime{ clock.m_currentTime }, 
				   m_deltaTime{ clock.m_deltaTime },
				   m_elapsedTime{ clock.m_elapsedTime },
				   m_elapsedUnscaledTime{ clock.m_elapsedUnscaledTime },
				   m_timeFrame{ clock.m_timeFrame },
				   m_timeScale{ clock.m_timeScale },
				   m_isPause{ clock.m_isPause } {}

  void Clock::updateTime() {
	++m_timeFrame;
	m_deltaTime = 0.0;

	if (!m_isPause) {
	  auto newTime = HighResClock::now();
	  auto delta = std::chrono::duration_cast<Nanoseconds>(newTime - m_currentTime);
	  auto elapse = std::chrono::duration_cast<Nanoseconds>(newTime - m_startTime);

	  m_currentTime = newTime;
	  m_deltaTime = delta.count() * 1e-9 * m_timeScale;
	  m_elapsedTime += m_deltaTime;
	  m_elapsedUnscaledTime = elapse.count() * 1e-9;
	}
  }

  double Clock::getDeltaTime() const { return m_deltaTime; }
  double Clock::getElapsedTime() const { return m_elapsedTime; }
  double Clock::getElapsedUnscaledTime() const { return m_elapsedUnscaledTime; }

  uint64_t Clock::getTimeFrame() const { return m_timeFrame; }

  uint64_t Clock::getTimestamp() {
	return std::chrono::seconds(std::time(nullptr)).count();
  }

};
