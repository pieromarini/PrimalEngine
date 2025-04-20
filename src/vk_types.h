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
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <vulkan/vk_enum_string_helper.h>

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
	glm::vec3 position;
	float uv_x;
	glm::vec3 normal;
	float uv_y;
	glm::vec4 color;
	glm::vec4 tangent;
};

// holds the resources needed for a mesh
struct GPUMeshBuffers {
	AllocatedBuffer indexBuffer;
	AllocatedBuffer vertexBuffer;
	VkDeviceAddress vertexBufferAddress;
};

// push constants for our mesh object draws
struct alignas(16) GPUDrawPushConstants {
	glm::vec4 viewPosition;
	glm::vec4 padding;
	glm::vec4 padding1;
	glm::vec4 padding2;
	VkDeviceAddress vertexBuffer;
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
