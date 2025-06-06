#pragma once

#include <cassert>
#include <cstdint>

#include <cstring>
#include <string>

namespace pm {

constexpr uint32_t STATIC_STRING_SIZE = 400;

struct PrimalString {
	PrimalString() = default;

	PrimalString(std::string s) : length(s.size()) {
		memcpy(data, s.data(), s.length());
	}

	PrimalString& operator=(std::string s) {
		assert(s.size() <= STATIC_STRING_SIZE);
		length = s.size();
		memcpy(data, s.data(), s.length());
		return *this;
	}

	PrimalString& operator=(PrimalString s) {
		length = s.length;
		memcpy(data, s.data, s.length);
		return *this;
	}

	// TODO(piero): bounds checking
	char& operator[](uint32_t index) {
		return data[index];
	}

	char operator[](uint32_t index) const {
		return data[index];
	}

	uint32_t length{};

	// TODO(piero): Have a pointer here and use an arena to allocate string?
	char data[STATIC_STRING_SIZE]{};
};

}// namespace pm
