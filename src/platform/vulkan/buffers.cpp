#include "buffers.h"
#include "vulkan_images.h"
#include "vulkan_renderer.h"
#include "vulkan_structures_helpers.h"

#include <vk_mem_alloc.h>

namespace pm {

AllocatedBuffer createBuffer(std::string name, size_t allocSize, VmaAllocator allocator, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
	// allocate buffer
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	AllocatedBuffer newBuffer{};

	// allocate the buffer
	VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info));

	vmaSetAllocationName(allocator, newBuffer.allocation, name.c_str());

	return newBuffer;
}

void destroyBuffer(VmaAllocator allocator, const AllocatedBuffer& buffer) {
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

AllocatedImage createImage(std::string name, VkExtent3D size, VkDevice device, VmaAllocator allocator, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
	AllocatedImage newImage{};
	newImage.imageFormat = format;
	newImage.imageExtent = size;

	VkImageCreateInfo img_info = imageCreateInfo(format, usage, size);
	if (mipmapped) {
		img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo allocinfo = {};
	allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	VK_CHECK(vmaCreateImage(allocator, &img_info, &allocinfo, &newImage.image, &newImage.allocation, nullptr));

	vmaSetAllocationName(allocator, newImage.allocation, name.c_str());

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build a image-view for the image
	VkImageViewCreateInfo view_info = imageViewCreateInfo(format, newImage.image, aspectFlag);
	view_info.subresourceRange.levelCount = img_info.mipLevels;

	VK_CHECK(vkCreateImageView(device, &view_info, nullptr, &newImage.imageView));

	return newImage;
}

AllocatedImage createImage(std::string name, void* data, VkExtent3D size, VulkanRendererContext* context, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
	size_t dataSize = size.depth * size.width * size.height * 4;
	AllocatedBuffer uploadbuffer = createBuffer("createImage uploadBuffer", dataSize, context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(uploadbuffer.info.pMappedData, data, dataSize);

	AllocatedImage newImage = createImage(name, size, context->device, context->vmaAllocator, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

	immediateSubmit(context, [&](VkCommandBuffer cmd) {
		transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = size;

		// copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		if (mipmapped) {
			generateMipmaps(cmd, newImage.image, { newImage.imageExtent.width, newImage.imageExtent.height });
		} else {
			transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	});

	// TOOD: we always default to this layout
	newImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	destroyBuffer(context->vmaAllocator, uploadbuffer);

	return newImage;
}

void destroyImage(VkDevice device, VmaAllocator allocator, const AllocatedImage& img) {
	vkDestroyImageView(device, img.imageView, nullptr);
	vmaDestroyImage(allocator, img.image, img.allocation);
}

};// namespace pm
