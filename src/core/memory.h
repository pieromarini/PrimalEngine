#ifndef MEMORY_H
#define MEMORY_H

#include <memory>

namespace primal {

  template<typename T>
  using Unique = std::unique_ptr<T>;

  template<typename T, typename ... Args>
  constexpr Unique<T> createUnique(Args&& ... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
  }

  template<typename T>
  using Shared = std::shared_ptr<T>;

  template<typename T, typename ... Args>
  constexpr Shared<T> createShared(Args&& ... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
  }

}

#endif
