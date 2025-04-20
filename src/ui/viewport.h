#pragma once

#include "ui/ui_types.h"
#include "vk_types.h"

namespace pm::UI {

struct Viewport {
	uint32_t id;
	float width, height;
	uint32_t textureId;
};

}// namespace pm::UI
