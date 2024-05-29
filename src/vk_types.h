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

#include <vulkan/vk_enum_string_helper.h>

namespace pm {

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

		// NOTE: sending pointer to vertex data as PushConstants for now.
		// We might want to set SSBOs using DescriptorSets instead.
		VkDeviceAddress vertexBuffer;
};

enum class MaterialPass : uint8_t {
	MainColor,
	Transparent,
	Other
};
struct MaterialPipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;
};

struct MaterialInstance {
		MaterialPipeline* pipeline;
		VkDescriptorSet materialSet;
		MaterialPass passType;
};

struct DrawContext;

// base class for a renderable dynamic object
class IRenderable {
		virtual void draw(const glm::mat4& topMatrix, DrawContext& ctx) = 0;
};

// implementation of a drawable scene node.
// the scene node can hold children and will also keep a transform to propagate
// to them
struct Node : public IRenderable {
		// parent pointer must be a weak pointer to avoid circular dependencies
		std::weak_ptr<Node> parent;
		std::vector<std::shared_ptr<Node>> children;

		glm::mat4 localTransform;
		glm::mat4 worldTransform;

		void refreshTransform(const glm::mat4& parentMatrix) {
			worldTransform = parentMatrix * localTransform;
			for (auto c : children) {
				c->refreshTransform(worldTransform);
			}
		}

		 void draw(const glm::mat4& topMatrix, DrawContext& ctx) override {
			// draw children
			for (auto& c : children) {
				c->draw(topMatrix, ctx);
			}
		}
};

#define VK_CHECK(x)                                                                  \
	do {                                                                               \
		VkResult err = x;                                                                \
		if (err) {                                                                       \
			std::cout << std::format("Detected Vulkan error: {}\n", string_VkResult(err)); \
			abort();                                                                       \
		}                                                                                \
	} while (0)

}
