#pragma once

#include "platform/vulkan/vulkan_descriptor.h"
#include "vk_types.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>
#include <filesystem>

namespace pm {

struct GLTFMaterial {
		MaterialInstance data;
};

struct GeoSurface {
		uint32_t startIndex;
		uint32_t count;
		std::shared_ptr<GLTFMaterial> material;
};

struct MeshAsset {
		std::string name;

		std::vector<GeoSurface> surfaces;
		GPUMeshBuffers meshBuffers;
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

		AllocatedBuffer materialDataBuffer;

		VulkanRenderer* renderer;

		~LoadedGLTF() { clearAll(); };

		virtual void draw(const glm::mat4& topMatrix, DrawContext& ctx);

	private:
		void clearAll();
};

std::optional<AllocatedImage> loadImage(VulkanRenderer* renderer, fastgltf::Asset& asset, fastgltf::Image& image);

std::optional<std::shared_ptr<LoadedGLTF>> loadGltf(VulkanRenderer* renderer, std::string_view filePath);


}// namespace pm
