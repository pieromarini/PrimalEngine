#include "material_config.h"
#include <format>
#include <simdjson.h>
#include <vulkan/vulkan_core.h>

namespace pm {

static ShaderStage stringToShaderStage(std::string_view stage) {
	if (stage == "vertex") {
		return SHADER_STAGE_VERTEX_BIT;
	} else if (stage == "fragment") {
		return SHADER_STAGE_FRAGMENT_BIT;
	} else if (stage == "compute") {
		return SHADER_STAGE_COMPUTE_BIT;
	}

	return (ShaderStage)0;
}

static DescriptorBindingType stringToBindingType(std::string_view stage) {
	if (stage == "uniform") {
		return DESCRIPTOR_BINDING_TYPE_UNIFORM_BUFFER;
	} else if (stage == "storage") {
		return DESCRIPTOR_BINDING_TYPE_STORAGE_BUFFER;
	} else if (stage == "storage_array") {
		return DESCRIPTOR_BINDING_TYPE_STORAGE_BUFFER;
	} else if (stage == "storage_image") {
		return DESCRIPTOR_BINDING_TYPE_STORAGE_IMAGE;
	} else if (stage == "combined_image_sampler") {
		return DESCRIPTOR_BINDING_TYPE_COMBINED_IMAGE_SAMPLER;
	} else if (stage == "bindless_image_samplers") {
		return DESCRIPTOR_BINDING_TYPE_COMBINED_IMAGE_SAMPLER;
	}

	return DESCRIPTOR_BINDING_TYPE_MAX_ENUM;
}

static PipelineType stringToPipelineType(std::string_view type) {
	if (type == "graphics") {
		return PIPELINE_TYPE_GRAPHICS;
	} else if (type == "compute") {
		return PIPELINE_TYPE_COMPUTE;
	}

	return PIPELINE_TYPE_NONE;
}

static ShaderType stringToShaderType(std::string_view type) {
	if (type == "vertex") {
		return SHADER_TYPE_VERTEX;
	} else if (type == "fragment") {
		return SHADER_TYPE_FRAGMENT;
	} else if (type == "compute") {
		return SHADER_TYPE_COMPUTE;
	}

	return SHADER_TYPE_NONE;
}

static VkCullModeFlags stringToCullMode(std::string_view mode) {
	if (mode == "none") {
		return VK_CULL_MODE_NONE;
	} else if (mode == "front") {
    return VK_CULL_MODE_FRONT_BIT;
	} else if (mode == "back") {
    return VK_CULL_MODE_BACK_BIT;
	} else if (mode == "double") {
    return VK_CULL_MODE_FRONT_AND_BACK;
	}

	return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}

static VkFrontFace stringToFrontFace(std::string_view mode) {
	if (mode == "clockwise") {
		return VK_FRONT_FACE_CLOCKWISE;
	} else if (mode == "counter_clockwise") {
    return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	return VK_FRONT_FACE_MAX_ENUM;
}

static BlendMode stringToBlendMode(std::string_view mode) {
	if (mode == "disabled") {
    return BLEND_MODE_DISABLED;
	} else if (mode == "additive") {
		return BLEND_MODE_ADDITIVE;
	} else if (mode == "alphablend") {
    return BLEND_MODE_ALPHABLEND;
	} else if (mode == "background") {
    return BLEND_MODE_BACKGROUND;
	}

  return BLEND_MODE_DISABLED;
}

MaterialConfig loadMaterialConfig(std::string_view path) {
	MaterialConfig config;

	simdjson::ondemand::parser parser;
	auto json = simdjson::padded_string::load(path);
	simdjson::ondemand::document doc = parser.iterate(json);

	auto root = doc.get_object();

	config.debugName = std::string(root["name"].get_string().value());

	// Pipeline config
	auto pipelineConfig = root["pipeline"].get_object();
	config.pipelineConfig.type = stringToPipelineType(pipelineConfig["type"].get_string().value());
	if (config.pipelineConfig.type == PIPELINE_TYPE_GRAPHICS) {
		config.pipelineConfig.cullMode = stringToCullMode(pipelineConfig["cull_mode"].get_string().value());
		config.pipelineConfig.frontFace = stringToFrontFace(pipelineConfig["front_face"].get_string().value());

		auto blendMode = pipelineConfig["blend_mode"];
		config.pipelineConfig.blendMode = blendMode.error() != simdjson::NO_SUCH_FIELD ? stringToBlendMode(blendMode) : BLEND_MODE_DISABLED;

		auto depthTest = pipelineConfig["depth_test"];
		config.pipelineConfig.depthTest = depthTest.error() != simdjson::NO_SUCH_FIELD ? depthTest : false;
	}

	for (auto shaderCfg : pipelineConfig["shaders"].get_array()) {
		ShaderConfig shaderConfig;
		shaderConfig.type = stringToShaderType(shaderCfg["type"].get_string().value());
		shaderConfig.path = std::string(shaderCfg["path"].get_string().value());

		// TODO(piero): refactor this
		if (shaderConfig.type == SHADER_TYPE_VERTEX) {
			config.pipelineConfig.shaders.at(0) = shaderConfig;	
		} else if (shaderConfig.type == SHADER_TYPE_FRAGMENT) {
			config.pipelineConfig.shaders.at(1) = shaderConfig;	
		} else if (shaderConfig.type == SHADER_TYPE_COMPUTE) {
			config.pipelineConfig.shaders.at(2) = shaderConfig;	
		} else {
			std::cout << std::format("Skipping shader: {}. Unknown shader type {}.\n", shaderConfig.path, (uint32_t)shaderConfig.type);
			continue;
		}
	}

	// Push Constants
	auto pushConstantsConfig = root["push_constants"].get_object();

	ShaderStageFlags stages{};
	for (std::string_view stage : pushConstantsConfig["stages"].get_array()) {
		stages |= stringToShaderStage(stage);
	}

	config.pushConstants = {
		.debugName = std::string(pushConstantsConfig["debug_name"].get_string().value()),
		.size = (uint32_t)pushConstantsConfig["size_in_bytes"].get_uint64()
	};
	config.pushConstants.stages = stages;

	// Descriptor Layout
	auto layoutsConfig = root["layouts"].get_array();
	for (auto layoutConfig : layoutsConfig) {
		auto set = (uint32_t)layoutConfig["set"].get_uint64();
		auto isGlobal = layoutConfig["global"];
		auto bindlessName = layoutConfig["bindless_name"];

		DescriptorLayout layout {
			.set = set,
			.isGlobal = isGlobal.error() != simdjson::NO_SUCH_FIELD ? isGlobal : false,
			.bindlessName = bindlessName.error() != simdjson::NO_SUCH_FIELD ? std::string(bindlessName.get_string().value()) : ""
		};
		for (auto bindingConfig : layoutConfig["bindings"].get_array()) {
			DescriptorBinding binding {
				.debugName = std::string(bindingConfig["debug_name"].get_string().value()),
				.binding = (uint32_t)bindingConfig["binding"].get_uint64(),
				.type = stringToBindingType(bindingConfig["type"].get_string().value()),
			};

			auto readonly = bindingConfig["readonly"];
			binding.readonly = readonly.error() != simdjson::NO_SUCH_FIELD ? readonly : false;

			uint64_t size{};
			auto err = bindingConfig["size_in_bytes"].get(size);
			binding.size = !err ? (uint32_t)size : 0;

			// TODO(piero): add sampler type to descriptor binding
			// auto samplerType = bindingConfig["sampler_type"];
			// binding.samplerType = !err ? samplerType : 0;

			for (std::string_view stage : bindingConfig["stages"].get_array()) {
				binding.stages |= stringToShaderStage(stage);
			}
			layout.bindings.push_back(binding);
		}
		config.layouts.push_back(layout);
	}

	return config;
}

};
