#pragma once

#include "../../utils/fileLoader.h"
#include <vulkan/vulkan.hpp>

namespace primal {

inline vk::ShaderModule createShaderModule(vk::Device device, const std::vector<char>& code) {
	vk::ShaderModuleCreateInfo createInfo{
		{},
		code.size(),
		reinterpret_cast<const uint32_t*>(code.data())
	};
	return device.createShaderModule(createInfo);
}

}// namespace primal
