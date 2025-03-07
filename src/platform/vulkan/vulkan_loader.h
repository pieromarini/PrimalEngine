#pragma once

#include "platform/vulkan/vulkan_descriptor.h"
#include "vk_types.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <filesystem>
#include <utility>

namespace pm {

struct GLTFMaterial {
	GLTFMaterial() = default;
	GLTFMaterial(std::string n, const MaterialInstance& d) : name{ std::move(n) }, data{ d } {}

	std::string name;
	MaterialInstance data;
};

struct GeoSurface {
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t indexCount;
	std::shared_ptr<GLTFMaterial> material;
};

struct MeshAsset {
	std::string name;
	std::vector<GeoSurface> surfaces;
};


struct AllocatedImage;
class VulkanRenderer;

std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(pm::VulkanRenderer* engine, std::filesystem::path filePath);

struct LoadedGLTF : public IRenderable {
	// storage for all the data on a given glTF file
	std::unordered_map<std::string, std::shared_ptr<MeshAsset>> meshes;
	std::unordered_map<std::string, std::shared_ptr<Node>> nodes;
	std::unordered_map<std::string, AllocatedImage> images;
	std::unordered_map<std::string, std::shared_ptr<GLTFMaterial>> materials;

	// nodes that dont have a parent, for iterating through the file in tree order
	std::vector<std::shared_ptr<Node>> topNodes;

	std::vector<VkSampler> samplers;

	DescriptorAllocator descriptorPool;

	GPUMeshBuffers modelBuffers;
	AllocatedBuffer materialDataBuffer;

	VulkanRenderer* renderer;

	virtual ~LoadedGLTF() { clearAll(); };

	void draw(const glm::mat4& topMatrix, DrawContext& ctx) override;

private:
	void clearAll();
};

std::optional<AllocatedImage> loadImage(VulkanRenderer* renderer, fastgltf::Asset& asset, fastgltf::Image& image);

std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanRenderer* renderer, std::string_view filePath);


}// namespace pm
