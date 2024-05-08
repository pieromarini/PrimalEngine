#include <fstream>

#include "vulkan_shader.h"
#include "vulkan_structures_helpers.h"

namespace pm {
bool loadShaderModule(const char* filePath, VkDevice device, VkShaderModule* outShaderModule) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return false;
	}

	auto fileSize = static_cast<size_t>(file.tellg());

	// spirv expects the buffer to be on uint32, so make sure to reserve a int
	// vector big enough for the entire file
	std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

	// put file cursor at beginning
	file.seekg(0);

	// load the entire file into the buffer
	file.read((char*)buffer.data(), fileSize);

	// now that the file is loaded into the buffer, we can close it
	file.close();

	// create a new shader module, using the buffer we loaded
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;

	// codeSize has to be in bytes, so multply the ints in the buffer by size of
	// int to know the real size of the buffer
	createInfo.codeSize = buffer.size() * sizeof(uint32_t);
	createInfo.pCode = buffer.data();

	// check that the creation goes well.
	VkShaderModule shaderModule{};
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		return false;
	}
	*outShaderModule = shaderModule;
	return true;
}
}// namespace pm
