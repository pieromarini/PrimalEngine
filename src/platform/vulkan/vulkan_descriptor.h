#pragma once

#include "vk_types.h"

struct DescriptorLayoutBuilder {
		void addBinding(uint32_t binding, VkDescriptorType type);
		void clear();
		VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

		std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct DescriptorAllocator {
		struct PoolSizeRatio {
				VkDescriptorType type;
				float ratio;
		};

		void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		void clearDescriptors(VkDevice device);
		void destroyPool(VkDevice device);

		VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);

		VkDescriptorPool pool;
};
