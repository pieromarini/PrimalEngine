#include "handle_bin.h"
#include "tools/log.h"
#include <limits>

namespace primal {

  HandleBin::HandleBin() : m_topHandle{ 1ll }, m_maxHandle{ std::numeric_limits<uint64_t>::max() } {}

  HandleBin::HandleBin(uint64_t start) : m_topHandle{ start }, m_maxHandle{ std::numeric_limits<uint64_t>::max() } { }

  HandleBin::HandleBin(uint64_t start, uint64_t max) : m_topHandle{ start }, m_maxHandle{ max } { }

  uint64_t HandleBin::getHandle() {
	if (m_handles.size() == 0) {
	  if (m_topHandle > m_maxHandle) {
		PRIMAL_CORE_ERROR("DataStructures::HandleBin::getHandle: Index out of range when trying to get handle.");
	  }
	  return m_topHandle++;
	} else {
	  int handle = *m_handles.begin();
	  m_handles.erase(handle);
	  return handle;
	}
  }

  void HandleBin::returnHandle(uint64_t handle) {
	if (handle >= m_topHandle || m_handles.find(handle)!= m_handles.end()) {
	  PRIMAL_CORE_ERROR("DataStructures::HandleBin::returnHandle: Trying to return an unused handle");
	}

	m_handles.insert(handle);
  }

  bool HandleBin::removeHandle(uint64_t handle) {
	if (handle > m_maxHandle) {
	  PRIMAL_CORE_ERROR("DataStructures::HandleBin::removeHandle: Trying to remove a greater handle than the max allowed.");
	}

	if (m_topHandle > handle && m_handles.find(handle) == m_handles.end()) {
	  return false;
	} else {
	  while (m_topHandle <= handle) {
		m_handles.insert(m_topHandle++);
	  }
	  m_handles.erase(handle);
	  return true;
	}
  }

  void HandleBin::clear() {
	m_handles.clear();
	m_topHandle = 1ll;
  }

}
