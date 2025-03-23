#pragma once

#include "vk_types.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
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

	MaterialPipeline* pipeline;
	MaterialPass passType;
};

Material Material_getDefaultMaterial();

}// namespace pm
