#include "platform/vulkan/vulkan_renderer.h"
#include "material.h"

namespace pm {

MaterialInstance createMaterialInstance(PrimalMaterial* material) {
	MaterialInstance instance{
		.material = material
	};

	// TODO(piero): This seems very hacky. We are "moving" all descriptor sets from
	//              the base material (basically, all global descriptor sets) to the instance.
	instance.descriptorSets.resize(material->layouts.size());

	for (uint32_t i = 0; i < material->layouts.size(); ++i) {
		auto layout = material->layouts.at(i);
		instance.descriptorSets.at(i) = layout.descriptorSet;
	}

	return instance;
}

// TODO(piero): Batch these together
void writeUniform(VulkanRendererContext* context, MaterialInstance* material, uint32_t set, uint32_t binding, VkBuffer buffer, uint32_t size, uint32_t offset) {
	VkDescriptorBufferInfo info = {
		.buffer = buffer,
		.offset = offset,
		.range = size
	};

	VkWriteDescriptorSet write = { .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstBinding = binding;
	write.dstSet = material->descriptorSets.at(set);
	write.descriptorCount = 1;
	write.descriptorType = (VkDescriptorType)material->material->layouts.at(set).bindings.at(binding).type;
	write.pBufferInfo = &info;

	vkUpdateDescriptorSets(context->device, 1, &write, 0, nullptr);
}

void writeUniform(VulkanRendererContext* context, MaterialInstance* material, uint32_t set, uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout, uint32_t index) {
	VkDescriptorImageInfo imageInfo = {
		.sampler = sampler,
		.imageView = imageView,
		.imageLayout = imageLayout
	};

	VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstSet = material->descriptorSets.at(set);
	write.dstBinding = binding;
	write.dstArrayElement = index;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(context->device, 1, &write, 0, nullptr);
}

}// namespace pm
