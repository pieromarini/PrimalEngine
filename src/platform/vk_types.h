#pragma once

#include <array>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include <format>

#include <vulkan/vulkan.h>

#include <vk_mem_alloc.h>

#include <vulkan/vk_enum_string_helper.h>

#include "core/math/math.h"

namespace pm {

struct AllocatedBuffer {
	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo info;
};

struct AllocatedImage {
	VkImage image;
	VkImageView imageView;
	VmaAllocation allocation;
	VkExtent3D imageExtent;
	VkFormat imageFormat;
	VkImageLayout imageLayout;
	VkSampler sampler;
	uint32_t mipLevels;
};

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
	vec4 tangent;
};

// holds the resources needed for a mesh
struct GPUMeshBuffers {
	AllocatedBuffer indexBuffer;
	AllocatedBuffer vertexBuffer;
	VkDeviceAddress vertexBufferAddress;
};

// push constants for our mesh object draws
struct alignas(16) GPUDrawPushConstants {
	vec4 viewPosition;
	vec4 padding;
	vec4 padding1;
	vec4 padding2;
	VkDeviceAddress vertexBuffer;
};

struct FontUniformData {
	mat4 projection;
	mat4 view;
	float pxRange;
};

struct UIUniformData {
	mat4 projection;
	mat4 view;
};


#define VK_CHECK(x)                                                                  \
	do {                                                                               \
		VkResult err = x;                                                                \
		if (err) {                                                                       \
			std::cout << std::format("Detected Vulkan error: {}\n", string_VkResult(err)); \
			abort();                                                                       \
		}                                                                                \
	} while (0)

}// namespace pm
