#pragma once

#include "vk_types.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>

#include "entity.h"

namespace pm {

struct AllocatedImage;
struct VulkanRendererContext;

struct Model {
	GPUMeshBuffers modelBuffers;// TODO: this should be moved out.
	Entity* root;
	std::vector<AllocatedImage> images;
	std::vector<VkSampler> samplers;
	std::vector<Material> materials;
};


std::optional<Model> loadGLTF(VulkanRendererContext* context, std::string_view filePath);
void cleanupModel(VulkanRendererContext* context, Model& model);
void cleanupModelEntities(Entity* root);

std::optional<AllocatedImage> loadImage(VulkanRendererContext* renderer, fastgltf::Asset& asset, fastgltf::Image& image);

}// namespace pm
