#ifndef CLOCK_H
#define CLOCK_H

#include <chrono>

namespace primal {

  class Clock {
	public:
	  Clock();
	  Clock(const Clock& clock);
	  Clock(Clock&& clock) noexcept;

	  Clock& operator=(const Clock& clock) = delete;
	  Clock& operator=(Clock&& clock) = delete;

	  float m_timeScale;
	  bool m_isPause;

	  void updateTime();

	  [[nodiscard]] double getDeltaTime() const;
	  [[nodiscard]] double getElapsedTime() const;

	  [[nodiscard]] double getElapsedUnscaledTime() const;

	  [[nodiscard]] uint64_t getTimeFrame() const;

	  static uint64_t getTimestamp();

	private:
	  using Nanoseconds = std::chrono::nanoseconds;
	  using HighResClock = std::chrono::high_resolution_clock;
	  using TimePoint  = std::chrono::time_point<HighResClock>;

	  TimePoint m_startTime;
	  TimePoint m_currentTime;

	  double m_deltaTime;
	  double m_elapsedTime;
	  double m_elapsedUnscaledTime;

	  uint64_t m_timeFrame;
  };

}

#endif
