#pragma once

#include "vk_types.h"

namespace pm {
	bool loadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);
}
