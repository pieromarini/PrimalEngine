#pragma once

#include "vk_types.h"

namespace pm {

inline AllocatedBuffer createBuffer(std::string name, size_t allocSize, VmaAllocator allocator, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
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

inline void destroyBuffer(VmaAllocator allocator, const AllocatedBuffer& buffer) {
	vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
}

}// namespace pm
