#pragma once

#include "vk_types.h"

struct DescriptorLayoutBuilder {
		void add_binding(uint32_t binding, VkDescriptorType type);
		void clear();
		VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

		std::vector<VkDescriptorSetLayoutBinding> bindings;
};
