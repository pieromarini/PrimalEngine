#pragma once

#include "renderer/material.h"
#include "vk_types.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace pm {

using MaterialIndex = uint32_t;

enum class MaterialPass : uint8_t {
	MainColor,
	Transparent,
	DoubleSided,
	Other
};

struct MaterialPipeline {
	VkPipeline pipeline;
	VkPipelineLayout layout;
};

// NOTE: this struct is sent to the shader as global material data
struct alignas(16) MaterialData {
	uint32_t albedoTexture{};
	uint32_t normalTexture{};
	uint32_t specularTexture{};
	uint32_t emissiveTexture{};
	glm::vec4 colorFactors;
	glm::vec4 metalRoughFactors;
};

struct Material {
	std::string name;
	MaterialData materialData;

	MaterialInstance material;
	// MaterialPipeline* pipeline;
	MaterialPass passType;
};

using MaterialCache = std::unordered_map<MaterialIndex, Material>;

Material Material_getDefaultMaterial();

MaterialCache MaterialCache_init();
bool MaterialCache_add(MaterialCache& cache, MaterialIndex index, Material material);
bool MaterialCache_remove(MaterialCache& cache, MaterialIndex index);
uint32_t MaterialCache_size(MaterialCache& cache);

Material& MaterialCache_get(MaterialCache& cache, MaterialIndex index);

}// namespace pm
