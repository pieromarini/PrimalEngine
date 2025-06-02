#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <vulkan/vulkan_core.h>

#include "material.h"

namespace pm {

struct PushConstantsConfig {
	std::string debugName;
	uint32_t size;
	ShaderStageFlags stages;
};

enum ShaderType: uint32_t {
	SHADER_TYPE_NONE = 0,
	SHADER_TYPE_VERTEX,
	SHADER_TYPE_FRAGMENT,
	SHADER_TYPE_COMPUTE
};

struct ShaderConfig {
	ShaderType type;
	std::string path;
};

enum PipelineType: uint32_t {
	PIPELINE_TYPE_NONE = 0,
	PIPELINE_TYPE_GRAPHICS,
	PIPELINE_TYPE_COMPUTE
};

// TODO(piero): revise names. We should also allow more customization.
//              e.g: blend factors, blend ops.
enum BlendMode: uint32_t {
	BLEND_MODE_DISABLED = 0,
	BLEND_MODE_ADDITIVE,
	BLEND_MODE_ALPHABLEND,
	BLEND_MODE_BACKGROUND
};

struct PipelineConfig {
	PipelineType type;
	VkCullModeFlags cullMode;
	VkFrontFace frontFace;
	BlendMode blendMode;
	bool depthTest; // TODO(piero): allow to configure COMPARE_OP and other settings

	// TODO(piero): refactor this
	/*
	 * 0 - Vertex
	 * 1 - Fragment
	 * 2 - Compute
	 */
	std::array<ShaderConfig, 3> shaders{};
};

struct MaterialConfig {
	std::string debugName;
	PipelineConfig pipelineConfig;
	PushConstantsConfig pushConstants;
	std::vector<DescriptorLayout> layouts;
};

MaterialConfig loadMaterialConfig(std::string_view path);

}// namespace pm
