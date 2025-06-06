#pragma once

#include <cstdint>

namespace pm::UI {

struct Viewport {
	uint32_t id;
	float width, height;
	uint32_t textureId;
};

}// namespace pm::UI
