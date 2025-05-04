#pragma once

#include "vk_types.h"

namespace pm {

struct VulkanRendererContext;

AllocatedBuffer createBuffer(std::string name, size_t allocSize, VmaAllocator allocator, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

void destroyBuffer(VmaAllocator allocator, const AllocatedBuffer& buffer);

AllocatedImage createImage(std::string name, VkExtent3D size, VkDevice device, VmaAllocator allocator, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);

AllocatedImage createImage(std::string name, void* data, VkExtent3D size, VulkanRendererContext* context, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);

void destroyImage(VkDevice device, VmaAllocator allocator, const AllocatedImage& img);

}// namespace pm
