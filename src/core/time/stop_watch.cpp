#include "stop_watch.h"

namespace primal {
  

  void StopWatch::start() {
	m_startTime = HighResClock::now();
  }

  float StopWatch::getSeconds() const {
	return Seconds(HighResClock::now() - m_startTime).count();
  }

  uint64_t StopWatch::getNanoseconds() const {
	return Nanoseconds(HighResClock::now() - m_startTime).count();
  }

  void StopWatch::reset() {
	m_startTime = HighResClock::now();
  }

}
