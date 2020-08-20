#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>

namespace primal {

  class StopWatch {
	public:
	  StopWatch() = default;
	  StopWatch(const StopWatch& watch) = delete;
	  StopWatch(StopWatch&& watch) = delete;

	  StopWatch& operator=(const StopWatch& watch) = delete;
	  StopWatch& operator=(StopWatch&& watch) = delete;

	  void start();
	  [[nodiscard]] float getSeconds() const;
	  [[nodiscard]] uint64_t getNanoseconds() const;
	  void reset();

	private:
	  using Seconds = std::chrono::duration<double>;
	  using Nanoseconds = std::chrono::duration<double, std::nano>;
	  using HighResClock = std::chrono::high_resolution_clock;
	  using TimePoint  = std::chrono::time_point<HighResClock>;

	  TimePoint m_startTime;
  };

}

#endif
