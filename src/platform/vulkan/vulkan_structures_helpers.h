#pragma once

#include <vk_types.h>

inline VkCommandPoolCreateInfo commandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags) {
	VkCommandPoolCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.flags = flags;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	return createInfo;
}

inline VkCommandBufferAllocateInfo commandBufferAllocateInfo(VkCommandPool pool, uint32_t count) {
	VkCommandBufferAllocateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	createInfo.commandPool = pool;
	createInfo.commandBufferCount = count;
	createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	return createInfo;
}

inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0) {
	VkFenceCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	createInfo.flags = flags;

	return createInfo;
}

inline VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0) {
	VkSemaphoreCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	createInfo.flags = flags;

	return createInfo;
}

inline VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0) {
	VkCommandBufferBeginInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	createInfo.pInheritanceInfo = nullptr;
	createInfo.flags = flags;

	return createInfo;
}

inline VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask) {
	VkImageSubresourceRange subImage = {};

	subImage.aspectMask = aspectMask;
	subImage.baseMipLevel = 0;
	subImage.levelCount = VK_REMAINING_MIP_LEVELS;
	subImage.baseArrayLayer = 0;
	subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

	return subImage;
}

inline VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore) {
	VkSemaphoreSubmitInfo semSubmitInfo = {};

	semSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	semSubmitInfo.semaphore = semaphore;
	semSubmitInfo.stageMask = stageMask;
	semSubmitInfo.deviceIndex = 0;
	semSubmitInfo.value = 1;

	return semSubmitInfo;
}

inline VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd) {
	VkCommandBufferSubmitInfo info = {};

	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
	info.commandBuffer = cmd;
	info.deviceMask = 0;

	return info;
}

inline VkSubmitInfo2 submitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo) {
	VkSubmitInfo2 info = {};

	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
	info.waitSemaphoreInfoCount = waitSemaphoreInfo == nullptr ? 0 : 1;
	info.pWaitSemaphoreInfos = waitSemaphoreInfo;

	info.signalSemaphoreInfoCount = signalSemaphoreInfo == nullptr ? 0 : 1;
	info.pSignalSemaphoreInfos = signalSemaphoreInfo;

	info.commandBufferInfoCount = 1;
	info.pCommandBufferInfos = cmd;

	return info;
}

inline VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
	VkImageCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.imageType = VK_IMAGE_TYPE_2D;

	createInfo.format = format;
	createInfo.extent = extent;

	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;

	// for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	// optimal tiling, which means the image is stored on the best gpu format
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.usage = usageFlags;

	return createInfo;
}

inline VkImageViewCreateInfo imageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
	// build a image-view for the depth image to use for rendering
	VkImageViewCreateInfo createInfo = {};

	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.image = image;
	createInfo.format = format;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	createInfo.subresourceRange.aspectMask = aspectFlags;

	return createInfo;
}

inline VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo(VkShaderStageFlagBits stageFlags, VkShaderModule shaderModule) {
	VkPipelineShaderStageCreateInfo stageInfo{};
	stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageInfo.pNext = nullptr;
	stageInfo.stage = stageFlags;
	stageInfo.module = shaderModule;
	stageInfo.pName = "main";

	return stageInfo;
}

inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo() {
	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.pNext = nullptr;

	// empty defaults
	createInfo.flags = 0;
	createInfo.setLayoutCount = 0;
	createInfo.pSetLayouts = nullptr;
	createInfo.pushConstantRangeCount = 0;
	createInfo.pPushConstantRanges = nullptr;

	return createInfo;
}

inline VkRenderingAttachmentInfo attachmentInfo(VkImageView view, VkClearValue* clear, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/) {
	VkRenderingAttachmentInfo colorAttachment{};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.pNext = nullptr;

	colorAttachment.imageView = view;
	colorAttachment.imageLayout = layout;
	colorAttachment.loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	if (clear) {
		colorAttachment.clearValue = *clear;
	}

	return colorAttachment;
}

inline VkRenderingAttachmentInfo depthAttachmentInfo(VkImageView view, VkImageLayout layout /*= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL*/) {
	VkRenderingAttachmentInfo depthAttachment{};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.pNext = nullptr;

	depthAttachment.imageView = view;
	depthAttachment.imageLayout = layout;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.clearValue.depthStencil.depth = 0.f;

	return depthAttachment;
}

inline VkRenderingInfo renderingInfo(VkExtent2D renderExtent, VkRenderingAttachmentInfo* colorAttachment, VkRenderingAttachmentInfo* depthAttachment) {
	VkRenderingInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.pNext = nullptr;

	renderInfo.renderArea = VkRect2D{ VkOffset2D{ 0, 0 }, renderExtent };
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = colorAttachment;
	renderInfo.pDepthAttachment = depthAttachment;
	renderInfo.pStencilAttachment = nullptr;

	return renderInfo;
}
