#ifndef HANDLEBIN_H 
#define HANDLEBIN_H

#include <set>
#include <cstdint>

namespace primal {

  class HandleBin {
	public:
	  HandleBin();
	  HandleBin(uint64_t start);
	  HandleBin(uint64_t start, uint64_t max);

	  uint64_t getHandle();
	  void returnHandle(uint64_t handle);
	  bool removeHandle(uint64_t handle);
	  void clear();

	private:
	  std::set<uint64_t> m_handles;
	  uint64_t m_topHandle;
	  uint64_t m_maxHandle;
  };

}

#endif
