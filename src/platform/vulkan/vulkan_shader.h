#pragma once

#include <vulkan/vulkan.h>

namespace pm {
	bool loadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule);
}
