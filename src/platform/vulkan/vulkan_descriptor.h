#pragma once

#include "vk_types.h"

struct DescriptorLayoutBuilder {
		void addBinding(uint32_t binding, VkDescriptorType type);
		void clear();
		VkDescriptorSetLayout build(VkDevice device, VkShaderStageFlags shaderStages, void* pNext = nullptr, VkDescriptorSetLayoutCreateFlags flags = 0);

		std::vector<VkDescriptorSetLayoutBinding> bindings;
};

struct DescriptorAllocator {
	public:
		struct PoolSizeRatio {
				VkDescriptorType type;
				float ratio;
		};

		void init(VkDevice device, uint32_t initalSets, std::span<PoolSizeRatio> poolRatios);
		void clearPools(VkDevice device);
		void destroyPools(VkDevice device);

		VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout, void* pNext = nullptr);

	private:
		VkDescriptorPool createPool(VkDevice device, uint32_t setCount, std::span<PoolSizeRatio> poolRatios);
		VkDescriptorPool getPool(VkDevice device);

		std::vector<PoolSizeRatio> ratios;
		std::vector<VkDescriptorPool> fullPools;
		std::vector<VkDescriptorPool> readyPools;
		uint32_t setsPerPool;
		uint32_t MAX_SETS_PER_POOL = 4092;
};

struct DescriptorWriter {
		void writeImage(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
		void writeBuffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

		void clear();
		void updateSet(VkDevice device, VkDescriptorSet set);

		std::deque<VkDescriptorImageInfo> imageInfos;
		std::deque<VkDescriptorBufferInfo> bufferInfos;
		std::vector<VkWriteDescriptorSet> writes;
};
