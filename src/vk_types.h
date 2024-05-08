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

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <vk_mem_alloc.h>

#include "vk_enum_string_helper.h"

struct AllocatedBuffer {
		VkBuffer buffer;
		VmaAllocation allocation;
		VmaAllocationInfo info;
};

struct Vertex {
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
};

// holds the resources needed for a mesh
struct GPUMeshBuffers {
		AllocatedBuffer indexBuffer;
		AllocatedBuffer vertexBuffer;
		VkDeviceAddress vertexBufferAddress;
};

// push constants for our mesh object draws
struct GPUDrawPushConstants {
		glm::mat4 worldMatrix;
		VkDeviceAddress vertexBuffer;
};

#define VK_CHECK(x)                                                                \
	do {                                                                             \
		VkResult err = x;                                                              \
		if (err) {                                                                     \
			std::cout << std::format("Detected Vulkan error: {}\n", string_VkResult(err)); \
			abort();                                                                     \
		}                                                                              \
	} while (0)
