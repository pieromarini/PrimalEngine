#pragma once

#include "platform/vulkan/vulkan_descriptor.h"
#include "vk_types.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <filesystem>
#include <utility>

#include "entity.h"
#include "material.h"

namespace pm {

struct alignas(16) MeshDraw {
	glm::mat4 transform{};
	uint32_t materialIndex{};
	float padding[3]{ 0.0f, 0.0f, 0.0f };
};

struct AllocatedImage;
class VulkanRenderer;

struct MeshPrimitive {
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t indexCount;
	MaterialIndex materialIndex; 
	MaterialPass passType;
};

struct Mesh {
	std::string name;
	std::vector<MeshPrimitive> primitives;
};

struct Model {
	GPUMeshBuffers modelBuffers;
	std::vector<Mesh> meshes;
	std::vector<std::shared_ptr<Node>> nodes;
	std::vector<std::shared_ptr<Node>> topNodes;
	std::vector<AllocatedImage> images;
	std::vector<Material> materials;
	std::vector<VkSampler> samplers;
};

std::optional<Model> loadGLTF(VulkanRenderer* renderer, std::string_view filePath);
void drawModel(Model& model, const glm::mat4& topMatrix, DrawContext& ctx);
void cleanupModel(VulkanRenderer* renderer, Model& model);

std::optional<AllocatedImage> loadImage(VulkanRenderer* renderer, fastgltf::Asset& asset, fastgltf::Image& image);

}// namespace pm
