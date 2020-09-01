#include "memory_manager.h"
#include "tools/log.h"

namespace primal {

  MemoryManager* MemoryManager::instance;

  // 10 MB stack allocator.
  MemoryManager::MemoryManager() : lsrAndSceneAllocator{ 10 * 1024 * 1024 } {
	instance = this;
  }

  void MemoryManager::update() {

  }

  MemoryManager* MemoryManager::getInstance() {
	if (instance == nullptr) {
	  PRIMAL_CORE_ERROR("MemoryManager::getInstance: instance doesn't exist. Be sure to acces it after its initialization.");
	}
	return instance;
  }

}
