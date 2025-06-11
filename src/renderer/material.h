#pragma once

#include "core/math/math.h"
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace pm {

struct VulkanRendererContext;

enum ShaderStage : uint32_t {
	SHADER_STAGE_NONE_BIT = 0x00000000,
	SHADER_STAGE_VERTEX_BIT = 0x00000001,
	SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
	SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
	SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
	SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
	SHADER_STAGE_COMPUTE_BIT = 0x00000020,
	SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
	SHADER_STAGE_ALL = 0x7FFFFFFF,
	SHADER_STAGE_RAYGEN_BIT_KHR = 0x00000100,
	SHADER_STAGE_ANY_HIT_BIT_KHR = 0x00000200,
	SHADER_STAGE_CLOSEST_HIT_BIT_KHR = 0x00000400,
	SHADER_STAGE_MISS_BIT_KHR = 0x00000800,
	SHADER_STAGE_INTERSECTION_BIT_KHR = 0x00001000,
	SHADER_STAGE_CALLABLE_BIT_KHR = 0x00002000,
	SHADER_STAGE_TASK_BIT_EXT = 0x00000040,
	SHADER_STAGE_MESH_BIT_EXT = 0x00000080,
	SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI = 0x00004000,
	SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI = 0x00080000,
	SHADER_STAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
};
using ShaderStageFlags = uint32_t;


enum DescriptorBindingType : uint32_t {
	/*
	DESCRIPTOR_BINDING_NONE = 0,
	DESCRIPTOR_BINDING_UNIFORM_BUFFER,
	DESCRIPTOR_BINDING_STORAGE_BUFFER,
	DESCRIPTOR_BINDING_STORAGE_ARRAY,
	DESCRIPTOR_BINDING_STORAGE_IMAGE,
	DESCRIPTOR_BINDING_COMBINED_IMAGE_SAMPLER,
	DESCRIPTOR_BINDING_BINDLESS_IMAGE_SAMPLERS
	*/

	DESCRIPTOR_BINDING_TYPE_SAMPLER = 0,
	DESCRIPTOR_BINDING_TYPE_COMBINED_IMAGE_SAMPLER = 1,
	DESCRIPTOR_BINDING_TYPE_SAMPLED_IMAGE = 2,
	DESCRIPTOR_BINDING_TYPE_STORAGE_IMAGE = 3,
	DESCRIPTOR_BINDING_TYPE_UNIFORM_TEXEL_BUFFER = 4,
	DESCRIPTOR_BINDING_TYPE_STORAGE_TEXEL_BUFFER = 5,
	DESCRIPTOR_BINDING_TYPE_UNIFORM_BUFFER = 6,
	DESCRIPTOR_BINDING_TYPE_STORAGE_BUFFER = 7,
	DESCRIPTOR_BINDING_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
	DESCRIPTOR_BINDING_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
	DESCRIPTOR_BINDING_TYPE_INPUT_ATTACHMENT = 10,
	DESCRIPTOR_BINDING_TYPE_INLINE_UNIFORM_BLOCK = 1000138000,
	DESCRIPTOR_BINDING_TYPE_ACCELERATION_STRUCTURE_KHR = 1000150000,
	DESCRIPTOR_BINDING_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
	DESCRIPTOR_BINDING_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM = 1000440000,
	DESCRIPTOR_BINDING_TYPE_BLOCK_MATCH_IMAGE_QCOM = 1000440001,
	DESCRIPTOR_BINDING_TYPE_MUTABLE_EXT = 1000351000,
	DESCRIPTOR_BINDING_TYPE_INLINE_UNIFORM_BLOCK_EXT = DESCRIPTOR_BINDING_TYPE_INLINE_UNIFORM_BLOCK,
	DESCRIPTOR_BINDING_TYPE_MUTABLE_VALVE = DESCRIPTOR_BINDING_TYPE_MUTABLE_EXT,
	DESCRIPTOR_BINDING_TYPE_MAX_ENUM = 0x7FFFFFFF
};

struct DescriptorBinding {
	std::string debugName;
	uint32_t binding;
	DescriptorBindingType type;
	bool readonly;
	uint32_t size;
	ShaderStageFlags stages;
};

struct DescriptorLayout {
	uint32_t set;
	bool isGlobal{ false };
	std::string bindlessName;
	std::vector<DescriptorBinding> bindings;

	// TODO(piero): refactor. This is only used to store "global" descriptors
	VkDescriptorSet descriptorSet{ nullptr };
};

struct PrimalMaterial {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	std::vector<DescriptorLayout> layouts;
	std::vector<VkDescriptorSetLayout> descriptorLayouts;
};

struct MaterialInstance {
	PrimalMaterial* material;
	std::vector<VkDescriptorSet> descriptorSets;
};

// TODO(piero): Refactor this
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
	vec4 colorFactors;
	vec4 metalRoughFactors;
};

struct Material {
	std::string name;
	MaterialData materialData;

	MaterialInstance material;
	// MaterialPipeline* pipeline;
	MaterialPass passType;
};

MaterialInstance createMaterialInstance(PrimalMaterial* material);

void writeUniform(VulkanRendererContext* context, MaterialInstance* material, uint32_t set, uint32_t binding, VkBuffer buffer, uint32_t size, uint32_t offset);
void writeUniform(VulkanRendererContext* context, MaterialInstance* material, uint32_t set, uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t index = 0);

};
