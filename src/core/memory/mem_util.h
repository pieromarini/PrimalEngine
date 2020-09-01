#ifndef MEM_UTIL_H
#define MEM_UTIL_H

#include <cstdint>
#include <string>

namespace primal {

  class MemUtil {
	public:
	  static const uint8_t ALIGNMENT = 16;
	  static void checkAlignment(uint8_t alignment);

	  static void* alloc(std::size_t size, uint8_t alignment = ALIGNMENT);

	  static void free(void* memoryPtr);

	  template<typename T>
	  static std::string getNameForType() {
		std::string name = typeid(T).name();
		auto pos = name.find("::");
		if (pos != std::string::npos) {
		  name = name.substr(pos + 2);
		}

		pos = name.find(" * __ptr64");
		if (pos != std::string::npos) {
		  name = name.substr(0, pos);
		  name += "*";
		}

		return name;
	  }

  };

}

#endif
