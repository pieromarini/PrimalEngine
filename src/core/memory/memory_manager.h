#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "stack_allocator.h"

namespace primal {

  class MemoryManager {
	public:
	  struct MemoryConfig {};

	  template<typename T, typename... Args>
	  static T* newOnStack(Args&&...);

	private:
	  MemoryManager();
	  ~MemoryManager() = default;

	  void update();
	  static MemoryManager* getInstance();

	  static MemoryManager* instance;

	  friend class EngineLoop;

	  StackAllocator lsrAndSceneAllocator;
  };

  template<typename T, typename... Args>
  T* MemoryManager::newOnStack(Args&&... args) {
	return getInstance()->lsrAndSceneAllocator.New<T>(std::forward<Args>(args)...);
  }

}

#endif
