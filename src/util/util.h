#include <functional>
#include <utility>

namespace primal::util {

  struct PairHash {
	template<typename T, typename S>
	std::size_t operator()(std::pair<T, S> const& p) const {
	  return (std::hash<T>()(p.first) ^ std::hash<S>()(p.second) << sizeof(p.first) );
	}

	template<typename T, typename S>
	bool operator()(std::pair<T, S>& a, const std::pair<T, S>& b) const {
	  return (a.first == b.first && a.second == b.second);
	}
  };

}
