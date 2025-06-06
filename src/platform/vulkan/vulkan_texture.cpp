#include "vulkan_texture.h"
#include "platform/vulkan/vulkan_images.h"
#include "platform/vulkan/vulkan_structures_helpers.h"
#include "buffers.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <iostream>
#include <ktx.h>

namespace pm {
void getValidFilters(VkPhysicalDevice physicalDevice, VkFormat format, VkFilter* filter, VkSamplerMipmapMode* mipmapMode) {
	// Not all formats support linear filtering, so we need to adjust them if they don't
	if (*filter == VK_FILTER_NEAREST && (mipmapMode == nullptr || *mipmapMode == VK_SAMPLER_MIPMAP_MODE_NEAREST)) {
		return;
	}

	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

	if (!(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		*filter = VK_FILTER_NEAREST;
		if (mipmapMode) {
			*mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}
	}
}

std::optional<AllocatedImage> createKTX2Image(std::string_view imageName, VkDevice device, VkCommandPool commandPool, VkQueue copyQueue, VmaAllocator allocator, void* imageData, uint32_t imageDataSize, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout) {
	AllocatedImage allocatedImage{};

	ktxTexture2* ktxTex{};
	auto result = ktxTexture2_CreateFromMemory((uint8_t*)imageData, imageDataSize, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTex);
	if (result != KTX_SUCCESS) {
		std::cout << std::format("Could not load the requested KTX image file: {}\n", imageName);
		return {};
	}

	if (ktxTexture2_NeedsTranscoding(ktxTex)) {
		auto start = std::chrono::high_resolution_clock::now();
		result = ktxTexture2_TranscodeBasis(ktxTex, KTX_TTF_BC7_RGBA, 0); // TODO: get format dynamically?
		auto transcodeTime = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - start).count();
		if (result != KTX_SUCCESS) {
			std::cout << std::format("Could not transcode the input texture to the selected target format: {}\n", imageName);
			return {};
		}
		std::cout << std::format("Transcode time: {:.2f}ms\n", transcodeTime);
	}

	allocatedImage.mipLevels = ktxTex->numLevels;

	allocatedImage.imageExtent = {
		.width = ktxTex->baseWidth,
		.height = ktxTex->baseHeight,
		.depth = ktxTex->baseDepth
	};

	allocatedImage.imageFormat = static_cast<VkFormat>(ktxTex->vkFormat);

	AllocatedBuffer stagingBuffer = createBuffer("loadKTX2Texture uploadBuffer", ktxTex->dataSize, allocator, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	auto data = stagingBuffer.info.pMappedData;
	memcpy(data, ktxTex->pData, ktxTex->dataSize);

	// Setup buffer copy regions for each mip level
	std::vector<VkBufferImageCopy> bufferCopyRegions;

	for (uint32_t i = 0; i < allocatedImage.mipLevels; i++) {
		ktx_size_t offset = 0;
		auto result = ktxTexture2_GetImageOffset(ktxTex, i, 0, 0, &offset);
		assert(result == KTX_SUCCESS);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = ktxTex->baseWidth >> i;
		bufferCopyRegion.imageExtent.height = ktxTex->baseHeight >> i;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
	}

	VkImageCreateInfo imgInfo = {};
	imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imgInfo.pNext = nullptr;
	imgInfo.imageType = VK_IMAGE_TYPE_2D;
	imgInfo.format = format;
	imgInfo.mipLevels = allocatedImage.mipLevels;
	imgInfo.arrayLayers = 1;
	imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Set initial layout of the image to undefined
	imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imgInfo.extent = {
		.width = allocatedImage.imageExtent.width,
		.height = allocatedImage.imageExtent.height,
		.depth = 1
	};
	imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | imageUsageFlags;

	VmaAllocationCreateInfo allocinfo = {};
	allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK(vmaCreateImage(allocator, &imgInfo, &allocinfo, &allocatedImage.image, &allocatedImage.allocation, nullptr));

	auto commandAllocateInfo = commandBufferAllocateInfo(commandPool, 1);
	VkCommandBuffer copyCmd = nullptr;
	VK_CHECK(vkAllocateCommandBuffers(device, &commandAllocateInfo, &copyCmd));

	auto cmdBufferBeginInfo = commandBufferBeginInfo();
	VK_CHECK(vkBeginCommandBuffer(copyCmd, &cmdBufferBeginInfo));

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = allocatedImage.mipLevels;
	subresourceRange.layerCount = 1;

	// Image barrier for optimal image (target)
	// Optimal image will be used as destination for the copy
	setImageLayout(
		copyCmd,
		allocatedImage.image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy mip levels from staging buffer
	vkCmdCopyBufferToImage(
		copyCmd,
		stagingBuffer.buffer,
		allocatedImage.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data());

	// Change texture image layout to shader read after all mip levels have been copied
	setImageLayout(
		copyCmd,
		allocatedImage.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageLayout,
		subresourceRange);

	allocatedImage.imageLayout = imageLayout;

	VK_CHECK(vkEndCommandBuffer(copyCmd));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;

	VkFence fence = nullptr;
	VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
	// Submit to the queue
	VK_CHECK(vkQueueSubmit(copyQueue, 1, &submitInfo, fence));
	// Wait for the fence to signal that command buffer has finished executing
	VK_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));
	vkDestroyFence(device, fence, nullptr);

	// Cleanup staging buffer
	destroyBuffer(allocator, stagingBuffer);

	ktxTexture2_Destroy(ktxTex);

	// Create image view
	VkImageViewCreateInfo imageViewCreateInfo{};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.components = { .r = VK_COMPONENT_SWIZZLE_R, .g = VK_COMPONENT_SWIZZLE_G, .b = VK_COMPONENT_SWIZZLE_B, .a = VK_COMPONENT_SWIZZLE_A };
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.subresourceRange.levelCount = allocatedImage.mipLevels;
	imageViewCreateInfo.image = allocatedImage.image;

	VK_CHECK(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &allocatedImage.imageView));

	return allocatedImage;
}

uint32_t getMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) {
	VkPhysicalDeviceMemoryProperties memoryProperties{};
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				if (memTypeFound) {
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound) {
		*memTypeFound = false;
		return 0;
	} else {
		throw std::runtime_error("Could not find a matching memory type");
	}
}

void Texture::updateDescriptor() {
	descriptor.sampler = sampler;
	descriptor.imageView = view;
	descriptor.imageLayout = imageLayout;
}

void Texture::destroy(VkDevice device) {
	vkDestroyImageView(device, view, nullptr);
	vkDestroyImage(device, image, nullptr);
	if (sampler) {
		vkDestroySampler(device, sampler, nullptr);
	}
	vkFreeMemory(device, deviceMemory, nullptr);
}

ktxResult Texture::loadKTXFile(std::string filename, ktxTexture** target) {
	ktxResult result = KTX_SUCCESS;
	if (!std::filesystem::exists(filename)) {
		std::cout << std::format("Could not load texture from {} \n. File doesn't exist.", filename);
	}
	result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);
	return result;
}

void Texture2D::loadFromFile(std::string filename, VkFormat format, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue copyQueue, float maxAnisotropy, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout) {
	ktxTexture* ktxTexture = nullptr;
	ktxResult result = loadKTXFile(filename, &ktxTexture);
	assert(result == KTX_SUCCESS);

	width = ktxTexture->baseWidth;
	height = ktxTexture->baseHeight;
	mipLevels = ktxTexture->numLevels;

	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetDataSize(ktxTexture);

	// Get device properties for the requested texture format
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

	VkMemoryAllocateInfo memAllocInfo{};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	// Use a separate command buffer for texture loading
	auto commandAllocateInfo = commandBufferAllocateInfo(commandPool, 1);
	VkCommandBuffer copyCmd = nullptr;
	VK_CHECK(vkAllocateCommandBuffers(device, &commandAllocateInfo, &copyCmd));

	auto cmdBufferBeginInfo = commandBufferBeginInfo();
	VK_CHECK(vkBeginCommandBuffer(copyCmd, &cmdBufferBeginInfo));

	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingMemory = nullptr;

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = ktxTextureSize;

	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(device, stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, nullptr);

	VK_CHECK(vkAllocateMemory(device, &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK(vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t* data = nullptr;
	VK_CHECK(vkMapMemory(device, stagingMemory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, ktxTextureData, ktxTextureSize);
	vkUnmapMemory(device, stagingMemory);

	// Setup buffer copy regions for each mip level
	std::vector<VkBufferImageCopy> bufferCopyRegions;

	for (uint32_t i = 0; i < mipLevels; i++) {
		ktx_size_t offset = 0;
		KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
		assert(result == KTX_SUCCESS);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
		bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo{};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { .width = width, .height = height, .depth = 1 };
	imageCreateInfo.usage = imageUsageFlags;

	// Ensure that the TRANSFER_DST bit is set for staging
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VK_CHECK(vkCreateImage(device, &imageCreateInfo, nullptr, &image));

	vkGetImageMemoryRequirements(device, image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;

	memAllocInfo.memoryTypeIndex = getMemoryType(physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr);
	VK_CHECK(vkAllocateMemory(device, &memAllocInfo, nullptr, &deviceMemory));
	VK_CHECK(vkBindImageMemory(device, image, deviceMemory, 0));

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = 1;

	// Image barrier for optimal image (target)
	// Optimal image will be used as destination for the copy
	setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	// Copy mip levels from staging buffer
	vkCmdCopyBufferToImage(
		copyCmd,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data());

	// Change texture image layout to shader read after all mip levels have been copied
	this->imageLayout = imageLayout;
	setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageLayout,
		subresourceRange,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	if (copyCmd == VK_NULL_HANDLE) {
		return;
	}

	VK_CHECK(vkEndCommandBuffer(copyCmd));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;

	VkFence fence = nullptr;
	VK_CHECK(vkCreateFence(device, &fenceCreateInfo, nullptr, &fence));
	// Submit to the queue
	VK_CHECK(vkQueueSubmit(copyQueue, 1, &submitInfo, fence));
	// Wait for the fence to signal that command buffer has finished executing
	VK_CHECK(vkWaitForFences(device, 1, &fence, VK_TRUE, 100000000000));
	vkDestroyFence(device, fence, nullptr);

	// Clean up staging resources
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingMemory, nullptr);

	ktxTexture_Destroy(ktxTexture);

	// Create a default sampler
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;

	// Max level-of-detail should match mip level count
	samplerCreateInfo.maxLod = (float)mipLevels;

	samplerCreateInfo.maxAnisotropy = maxAnisotropy;
	samplerCreateInfo.anisotropyEnable = true;

	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VK_CHECK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &sampler));

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1 };
	viewCreateInfo.subresourceRange.levelCount = mipLevels;
	viewCreateInfo.image = image;
	VK_CHECK(vkCreateImageView(device, &viewCreateInfo, nullptr, &view));

	// Update descriptor image info member that can be used for setting up descriptor sets
	updateDescriptor();
}

}// namespace pm
