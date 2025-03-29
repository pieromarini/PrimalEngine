#pragma once

#include "vk_types.h"
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/parser.hpp>
#include <fastgltf/tools.hpp>

#include "entity.h"

namespace pm {

struct AllocatedImage;
class VulkanRenderer;

struct Model {
	GPUMeshBuffers modelBuffers;// TODO: this should be moved out.
	Entity* root;
	std::vector<AllocatedImage> images;
	std::vector<VkSampler> samplers;
	std::vector<Material> materials;
};


std::optional<Model> loadGLTF(VulkanRenderer* renderer, std::string_view filePath);
void cleanupModel(VulkanRenderer* renderer, Model& model);
void cleanupModelEntities(Entity* root);

std::optional<AllocatedImage> loadImage(VulkanRenderer* renderer, fastgltf::Asset& asset, fastgltf::Image& image);

}// namespace pm
