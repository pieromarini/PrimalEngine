#include "assets/image_loader.h"
#include "platform/vulkan/buffers.h"
#include "platform/window.h"
#include "primal.h"
#include "renderer/material_config.h"
#include "terrain/voxel.h"
#include "ui/ui_manager.h"

#include "vk_types.h"

#include <queue>
#include <vk_mem_alloc.h>

#include "assets/font_loader.h"
#include "config.h"
#include "memory/arena.h"
#include "memory/data_structures/fixed_array.h"
#include "swapchain.h"
#include "ui/ui_types.h"
#include "ui/widgets.h"
#include <chrono>
#include <ratio>
#include <vulkan/vulkan_core.h>


#include "entity.h"
#include "platform/vulkan/vulkan_descriptor.h"
#include "platform/vulkan/vulkan_images.h"
#include "platform/vulkan/vulkan_loader.h"
#include "ui/primitives.h"
#include "vulkan_pipeline.h"
#include "vulkan_renderer.h"
#include "vulkan_shader.h"
#include "vulkan_structures_helpers.h"
#include <array>
#include <cmath>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <vk_mem_alloc.h>


namespace pm {

void rendererInit(VulkanRendererContext* context) {
	initVulkan(context);
}

void rendererSetup(VulkanRendererContext* context) {
	rendererInitMemory(context);

	initRenderTargets(context);
	initCommands(context);
	initSyncStructures(context);
	initDescriptors(context);
	initPipelines(context);
	initQueryPools(context);

	rendererInitDefaultData(context);
	initFontData(context);
	initUI(context);


	// Default lighting parameters
	context->sceneData.ambientColor = glm::vec4(.4f);
	context->sceneData.sunlightColor = glm::vec4(1.f, 1.0, 1.0f, 1.0f);
	context->sceneData.sunlightDirection = glm::vec4(0.38f, 1.0f, 0.97f, 1.f);

	/*
	auto start = std::chrono::system_clock::now();
	context->voxelTerrain = generateTerrain();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
	std::cout << std::format("Generated terrain in {:.4f} ms\n", static_cast<float>(elapsed.count()));

	setupVoxelRaycastRenderer(context);
	*/

	// loadTestScene(context);
	terrainTest(context);
}

// NOTE(piero): not using this right now.
void rendererInitMemory(VulkanRendererContext* context) {
	for (auto& frame : context->frames) {
		// frame.perFrameArena = MemoryArena_create(MEGABYTE(256));
	}
}

PrimalMaterial createMaterial(VulkanRendererContext* context, std::string_view materialConfig, bool flag) {
	PrimalMaterial material;

	auto config = loadMaterialConfig(materialConfig);

	material.descriptorLayouts.reserve(config.layouts.size());

	// create descriptor set layouts
	for (auto& layout : config.layouts) {
		if (layout.isGlobal) {
			// NOTE(piero): need to refactor this. Very hacky. We are storing the descriptor set inside of the layout object
			// TODO(piero): We are not using the descriptor layout parsed from the material config file (layout.bindings)
			// NOTE(piero): Using a property on the materials called `bindless_name` we can "share" these bindless arrays between materials.
			//              This seems very hacky but it works for now. We initialize the bindless array once and just copy over the descriptor layout and set for the different materials.
			//              This allows us to write to the same descriptor set from different materials.
			auto set = context->bindlessTextureArrays.find(layout.bindlessName);
			if (set != context->bindlessTextureArrays.end()) {
				material.descriptorLayouts.push_back(set->second.first);
				layout.descriptorSet = set->second.second;
			} else {
				VkDescriptorSetLayout descriptorSetLayout{};
				initBindlessTextureDescriptor(context, descriptorSetLayout, layout.descriptorSet);
				material.descriptorLayouts.push_back(descriptorSetLayout);
			}
		} else {
			DescriptorLayoutBuilder builder;
			for (auto binding : layout.bindings) {
				builder.addBinding(binding.binding, (VkDescriptorType)binding.type, binding.stages);
			}
			material.descriptorLayouts.push_back(builder.build(context->device));
		}
	}

	// NOTE(piero): Copying array of layouts.
	material.layouts = config.layouts;

	VkPushConstantRange pushConstants{};
	pushConstants.offset = 0;
	pushConstants.size = config.pushConstants.size;
	pushConstants.stageFlags = config.pushConstants.stages;

	VkPipelineLayoutCreateInfo pipelineLayout = pipelineLayoutCreateInfo();
	pipelineLayout.pPushConstantRanges = &pushConstants;
	pipelineLayout.pushConstantRangeCount = 1;
	pipelineLayout.setLayoutCount = material.descriptorLayouts.size();
	pipelineLayout.pSetLayouts = material.descriptorLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(context->device, &pipelineLayout, nullptr, &material.pipelineLayout));

	if (config.pipelineConfig.type == PIPELINE_TYPE_COMPUTE) {
		VkShaderModule computeShader{};
		if (!loadShaderModule(config.pipelineConfig.shaders[2].path.c_str(), context->device, &computeShader)) {
			std::cout << std::format("Error when building the compute shader\n");
		}

		auto stageinfo = pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computeShader);

		VkComputePipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.pNext = nullptr;
		pipelineCreateInfo.layout = material.pipelineLayout;
		pipelineCreateInfo.stage = stageinfo;

		VK_CHECK(vkCreateComputePipelines(context->device, context->pipelineCache, 1, &pipelineCreateInfo, nullptr, &material.pipeline));

		vkDestroyShaderModule(context->device, computeShader, nullptr);

	} else if (config.pipelineConfig.type == PIPELINE_TYPE_GRAPHICS) {
		VkShaderModule vertexShader{};
		if (!loadShaderModule(config.pipelineConfig.shaders[0].path.c_str(), context->device, &vertexShader)) {
			std::cout << std::format("Error when building the vertex shader\n");
		}

		VkShaderModule fragmentShader{};
		if (!loadShaderModule(config.pipelineConfig.shaders[1].path.c_str(), context->device, &fragmentShader)) {
			std::cout << std::format("Error when building the fragment shader\n");
		}

		PipelineBuilder pipelineBuilder;
		pipelineBuilder.setPipelineLayout(material.pipelineLayout);
		pipelineBuilder.setShaders(vertexShader, fragmentShader);
		pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder.setCullMode(config.pipelineConfig.cullMode, config.pipelineConfig.frontFace);

		// TODO(piero): make configurable
		pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
		pipelineBuilder.setMultisamplingNone();

		switch (config.pipelineConfig.blendMode) {
		case BLEND_MODE_DISABLED:
			pipelineBuilder.disableBlending();
			break;
		case BLEND_MODE_ADDITIVE:
			pipelineBuilder.enableBlendingAdditive();
			break;
		case BLEND_MODE_ALPHABLEND:
			pipelineBuilder.enableBlendingAlphablend();
			break;
		case BLEND_MODE_BACKGROUND:
			pipelineBuilder.enableBackgroundBlending();
			break;
		}

		if (config.pipelineConfig.depthTest) {
			pipelineBuilder.enableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
		} else {
			pipelineBuilder.disableDepthTest();
		}

		// TODO(piero): make configurable.
		if (flag) {
			pipelineBuilder.setColorAttachmentFormat(context->sceneDrawImage.imageFormat);
			pipelineBuilder.setDepthFormat(context->sceneDepthImage.imageFormat);
		} else {
			pipelineBuilder.setColorAttachmentFormat(context->rendererState->window->renderTarget.imageFormat);
			pipelineBuilder.setDepthFormat(context->rendererState->window->depthTarget.imageFormat);
		}

		material.pipeline = pipelineBuilder.buildPipeline(context->device, context->pipelineCache);

		vkDestroyShaderModule(context->device, vertexShader, nullptr);
		vkDestroyShaderModule(context->device, fragmentShader, nullptr);
	} else {
		assert(!"Unknown pipeline config type");
	}

	return material;
}

void destroyMaterial(VulkanRendererContext* context, PrimalMaterial& material) {
	vkDestroyPipelineLayout(context->device, material.pipelineLayout, nullptr);
	vkDestroyPipeline(context->device, material.pipeline, nullptr);

	for (auto& descriptorLayout : material.descriptorLayouts) {
		vkDestroyDescriptorSetLayout(context->device, descriptorLayout, nullptr);
	}
}

void setupVoxelRaycastRenderer(VulkanRendererContext* context) {
	// TODO(piero): Move this out.
	auto imageAsset = loadPNG("BlueNoiseAsset", "res/textures/blue_noise_rgba.png");
	if (imageAsset.data) {
		VkExtent3D imagesize{};
		imagesize.width = imageAsset.width;
		imagesize.height = imageAsset.height;
		imagesize.depth = 1;

		context->blueNoise = createImage("BlueNoise", imageAsset.data, imagesize, context, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT, true);
		context->blueNoise.sampler = context->defaultSamplerLinear;
	}
	destroyImageAsset(imageAsset);

	createGBuffer(context);

	// gbuffer pipeline
	VkShaderModule computeDrawShader{};
	if (!loadShaderModule("res/shaders/voxel_dda.comp.spv", context->device, &computeDrawShader)) {
		std::cout << std::format("Error when building the compute shader \n");
	}

	VkPushConstantRange pushConstants{};
	pushConstants.offset = 0;
	pushConstants.size = sizeof(ComputePushConstants);
	pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	std::array<VkDescriptorSetLayout, 2> layouts = {
		context->gpuSceneDataDescriptorLayout,
		context->gbufferDescriptorLayout
	};

	VkPipelineLayoutCreateInfo gbufferLayout = pipelineLayoutCreateInfo();
	gbufferLayout.pPushConstantRanges = &pushConstants;
	gbufferLayout.pushConstantRangeCount = 1;
	gbufferLayout.setLayoutCount = layouts.size();
	gbufferLayout.pSetLayouts = layouts.data();

	VK_CHECK(vkCreatePipelineLayout(context->device, &gbufferLayout, nullptr, &context->gbufferPipelineLayout));

	auto stageinfo = pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = context->gbufferPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	VK_CHECK(vkCreateComputePipelines(context->device, context->pipelineCache, 1, &computePipelineCreateInfo, nullptr, &context->gbufferPipeline));

	vkDestroyShaderModule(context->device, computeDrawShader, nullptr);

	context->mainDeletionQueue.push([context]() {
		vkDestroyPipelineLayout(context->device, context->gbufferPipelineLayout, nullptr);
		vkDestroyPipeline(context->device, context->gbufferPipeline, nullptr);
	});

	// resolve gbuffer pipeline
	VkShaderModule resolveDrawShader{};
	if (!loadShaderModule("res/shaders/gbuffer_resolve.comp.spv", context->device, &resolveDrawShader)) {
		std::cout << std::format("Error when building the compute shader \n");
	}

	VkPushConstantRange pc{};
	pc.offset = 0;
	pc.size = sizeof(uint32_t);
	pc.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	std::array<VkDescriptorSetLayout, 1> resolveLayouts = {
		context->resolveDescriptorLayout
	};

	VkPipelineLayoutCreateInfo resolveLayout = pipelineLayoutCreateInfo();
	resolveLayout.pPushConstantRanges = &pc;
	resolveLayout.pushConstantRangeCount = 1;
	resolveLayout.setLayoutCount = resolveLayouts.size();
	resolveLayout.pSetLayouts = resolveLayouts.data();

	VK_CHECK(vkCreatePipelineLayout(context->device, &resolveLayout, nullptr, &context->resolvePipelineLayout));

	auto resolveStageInfo = pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, resolveDrawShader);

	VkComputePipelineCreateInfo resolvePipelineCreateInfo{};
	resolvePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	resolvePipelineCreateInfo.pNext = nullptr;
	resolvePipelineCreateInfo.layout = context->resolvePipelineLayout;
	resolvePipelineCreateInfo.stage = resolveStageInfo;

	VK_CHECK(vkCreateComputePipelines(context->device, context->pipelineCache, 1, &resolvePipelineCreateInfo, nullptr, &context->resolvePipeline));

	vkDestroyShaderModule(context->device, resolveDrawShader, nullptr);

	context->mainDeletionQueue.push([context]() {
		vkDestroyPipelineLayout(context->device, context->resolvePipelineLayout, nullptr);
		vkDestroyPipeline(context->device, context->resolvePipeline, nullptr);
	});
}

void createGBuffer(VulkanRendererContext* context) {
	// TODO(piero): Refactor viewport render targets resizing
	VkExtent3D size{
		.width = context->fullScreen ? static_cast<uint32_t>(context->rendererState->window->width) : 1448,
		.height = context->fullScreen ? static_cast<uint32_t>(context->rendererState->window->height - 80) : 700,
		.depth = 1
	};

	VkImageUsageFlags usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	context->gbuffer = {
		.albedo = createImage("GBuffer-Albedo", size, context->device, context->vmaAllocator, VK_FORMAT_R8G8B8A8_UNORM, usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
		.irradiance = createImage("GBuffer-Irradiance", size, context->device, context->vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, usage),
		.depth = createImage("GBuffer-Depth", size, context->device, context->vmaAllocator, VK_FORMAT_R32_SFLOAT, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT),

		.gridInfo = createBuffer("voxel grid buffer", sizeof(VoxelGrid), context->vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU),
		.voxelData = createBuffer("voxel data buffer", sizeof(uint32_t) * context->voxelTerrain.voxelData.size(), context->vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU)
	};

	memcpy(context->gbuffer.gridInfo.info.pMappedData, &context->voxelTerrain.grid, sizeof(VoxelGrid));
	memcpy(context->gbuffer.voxelData.info.pMappedData, context->voxelTerrain.voxelData.data(), sizeof(uint32_t) * context->voxelTerrain.voxelData.size());

	// descriptors
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		builder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		builder.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		builder.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		builder.addBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		builder.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		context->gbufferDescriptorLayout = builder.build(context->device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	context->gbufferDescriptorSet = context->globalDescriptorAllocator.allocate(context->device, context->gbufferDescriptorLayout);

	{
		DescriptorWriter writer;
		writer.writeImage(0, context->gbuffer.albedo.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writer.writeImage(1, context->gbuffer.irradiance.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writer.writeImage(2, context->gbuffer.depth.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writer.writeBuffer(3, context->gbuffer.gridInfo.buffer, sizeof(VoxelGrid), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writer.writeBuffer(4, context->gbuffer.voxelData.buffer, sizeof(uint32_t) * context->voxelTerrain.voxelData.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writer.writeImage(5, context->blueNoise.imageView, context->blueNoise.sampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.updateSet(context->device, context->gbufferDescriptorSet);
	}

	// resolve pipeline (write gbuffer to render target)

	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		builder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		builder.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		context->resolveDescriptorLayout = builder.build(context->device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	context->resolveDescriptorSet = context->globalDescriptorAllocator.allocate(context->device, context->resolveDescriptorLayout);

	{
		DescriptorWriter writer;
		writer.writeImage(0, context->gbuffer.albedo.imageView, context->defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.writeImage(1, context->gbuffer.irradiance.imageView, context->defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.writeImage(2, context->gbuffer.depth.imageView, context->defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.writeImage(3, context->sceneDrawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writer.updateSet(context->device, context->resolveDescriptorSet);
	}
}

void destroyGBuffer(VulkanRendererContext* context) {
	destroyImage(context->device, context->vmaAllocator, context->gbuffer.albedo);
	destroyImage(context->device, context->vmaAllocator, context->gbuffer.irradiance);
	destroyImage(context->device, context->vmaAllocator, context->gbuffer.depth);

	destroyBuffer(context->vmaAllocator, context->gbuffer.gridInfo);
	destroyBuffer(context->vmaAllocator, context->gbuffer.voxelData);

	vkDestroyDescriptorSetLayout(context->device, context->gbufferDescriptorLayout, nullptr);
	vkDestroyDescriptorSetLayout(context->device, context->resolveDescriptorLayout, nullptr);
}

void terrainTest(VulkanRendererContext* context) {
	vkDeviceWaitIdle(context->device);

	auto start = std::chrono::system_clock::now();
	context->voxelTerrain = generateTerrain(&context->terrainParams);
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
	std::cout << std::format("Generated terrain in {:.4f} ms\n", static_cast<float>(elapsed.count()));

	std::vector<VoxelVertex> vertices;
	std::vector<uint32_t> indices;

	// reserve space for terrain
	vertices.reserve(6 * 4 * context->voxelTerrain.voxelCount);
	indices.reserve(6 * 6 * context->voxelTerrain.voxelCount);

	start = std::chrono::system_clock::now();
	generateTerrainGeometry(context->voxelTerrain, vertices, indices);
	elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start);
	std::cout << std::format("Generated terrain geometry with {} voxels in {:.4f} ms\n", context->voxelTerrain.voxelCount, static_cast<float>(elapsed.count()));

	context->voxelMeshBuffers = uploadMesh(context, indices, vertices, "voxelMeshBuffers");

	std::cout << std::format("Terrain Memory Usage: Vertex {:.2f} MB | Indices {:.2f} MB\n", static_cast<double>(context->voxelMeshBuffers.vertexBuffer.info.size) * 1e-6, static_cast<double>(context->voxelMeshBuffers.indexBuffer.info.size) * 1e-6);
}

void cleanupTerrain(VulkanRendererContext* context) {
	destroyBuffer(context->vmaAllocator, context->voxelMeshBuffers.vertexBuffer);
	destroyBuffer(context->vmaAllocator, context->voxelMeshBuffers.indexBuffer);
}

void loadTestScene(VulkanRendererContext* context) {
	const std::string modelPath = { "res/models/bistro/bistro_ktx2.glb" };
	// const std::string modelPath = { "res/models/structure.glb" };

	auto start = std::chrono::system_clock::now();
	auto loadedGLTF = loadGLTF(context, modelPath);
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << std::format("Loaded in {:.4f} seconds\n", static_cast<float>(elapsed.count()) / 1000.0f);

	assert(loadedGLTF.has_value());

	context->loadedModels.push_back(loadedGLTF.value());
}

void rendererSetInitialState(VulkanRendererContext* context, VulkanRendererConfig* state) {
	context->rendererState = state;
}

void initQueryPools(VulkanRendererContext* context) {
	context->timestampPool = createQueryPool(context->device, 128, VK_QUERY_TYPE_TIMESTAMP);
	assert(context->timestampPool);

	context->pipelineStatisticsPool = createQueryPool(context->device, 1, VK_QUERY_TYPE_PIPELINE_STATISTICS);
	assert(context->pipelineStatisticsPool);

	context->mainDeletionQueue.push([context] {
		vkDestroyQueryPool(context->device, context->timestampPool, nullptr);
		vkDestroyQueryPool(context->device, context->pipelineStatisticsPool, nullptr);
	});
}

void resizeSwapchain(VulkanRendererContext* context, PrimalWindow* window) {
	vkDeviceWaitIdle(context->device);

	destroySwapchain(context->device, &window->swapchain);

	int newWidth{}, newHeight{};
	getWindowSize(window, &newWidth, &newHeight);

	int displayWidth{}, displayHeight{};
	getWindowSizeInPixels(window, &displayWidth, &displayHeight);

	context->renderScale = static_cast<float>(displayWidth) / static_cast<float>(newWidth);

	window->swapchain = createSwapchain(context->device, context->physicalDevice, window->surface, newWidth, newHeight, VK_FORMAT_B8G8R8A8_UNORM, VK_PRESENT_MODE_IMMEDIATE_KHR);

	// recreate window render target
	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkExtent3D newWindowExtent{
		static_cast<uint32_t>(newWidth),
		static_cast<uint32_t>(newHeight),
		1
	};

	// NOTE(piero): I just want to keep the same VMA allocation names for tracking purposes
	VmaAllocationInfo info{};
	vmaGetAllocationInfo(context->vmaAllocator, window->renderTarget.allocation, &info);
	auto nRT = info.pName;
	vmaGetAllocationInfo(context->vmaAllocator, window->depthTarget.allocation, &info);
	auto nDT = info.pName;

	destroyImage(context->device, context->vmaAllocator, window->renderTarget);
	destroyImage(context->device, context->vmaAllocator, window->depthTarget);
	window->renderTarget = createImage(nRT, newWindowExtent, context->device, context->vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, drawImageUsages, false);
	window->depthTarget = createImage(nDT, newWindowExtent, context->device, context->vmaAllocator, VK_FORMAT_D32_SFLOAT, depthImageUsages, false);

	UI::onResizeCallback(static_cast<float>(window->width), static_cast<float>(window->height));

	window->width = newWidth;
	window->height = newHeight;

	window->resizeRequested = false;
}

void resizeRenderTargets(VulkanRendererContext* context) {
	vkDeviceWaitIdle(context->device);

	VkExtent3D sceneDrawImageExtent{
		.width = context->fullScreen ? static_cast<uint32_t>(context->rendererState->window->width) : 1448,
		.height = context->fullScreen ? static_cast<uint32_t>(context->rendererState->window->height - 80) : 700,
		.depth = 1
	};

	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	destroyImage(context->device, context->vmaAllocator, context->sceneDrawImage);
	destroyImage(context->device, context->vmaAllocator, context->sceneDepthImage);
	context->sceneDrawImage = createImage("scene drawImage", sceneDrawImageExtent, context->device, context->vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, false);
	context->sceneDepthImage = createImage("scene depthImage", sceneDrawImageExtent, context->device, context->vmaAllocator, VK_FORMAT_D32_SFLOAT, depthImageUsages, false);

	// Overwrite registered image and write to descriptor.
	// TODO(piero): refactor
	context->registeredImages[context->sceneTextureId - 1] = &context->sceneDrawImage;
	writeUniform(context, &context->uiViewportMaterialInstance, 1, 0, context->sceneDrawImage.imageView, context->defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, context->sceneTextureId);
}

void rendererInitDefaultData(VulkanRendererContext* context) {
	// 3 default textures, white, grey, black. 1 pixel each
	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	context->whiteImage = createImage("whiteImage", (void*)&white, VkExtent3D{ 1, 1, 1 }, context, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	uint32_t grey = glm::packUnorm4x8(glm::vec4(0.66f, 0.66f, 0.66f, 1));
	context->greyImage = createImage("greyImage", (void*)&grey, VkExtent3D{ 1, 1, 1 }, context, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	context->blackImage = createImage("blackImage", (void*)&black, VkExtent3D{ 1, 1, 1 }, context, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	// checkerboard image
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	std::array<uint32_t, 16 * 16> pixels{};// for 16x16 checkerboard texture
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			pixels.at(y * 16 + x) = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}
	context->errorCheckerboardImage = createImage("errorCheckedboardImage", pixels.data(), VkExtent3D{ 16, 16, 1 }, context, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	VkSamplerCreateInfo sampl = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

	sampl.magFilter = VK_FILTER_NEAREST;
	sampl.minFilter = VK_FILTER_NEAREST;

	vkCreateSampler(context->device, &sampl, nullptr, &context->defaultSamplerNearest);

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	vkCreateSampler(context->device, &sampl, nullptr, &context->defaultSamplerLinear);

	// Create global material data buffer
	// TODO: Using fixed size for now.
	context->globalMaterialDataBuffer = createBuffer("globalMaterialDataBuffer", sizeof(MaterialData) * 1001, context->vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	auto sceneMaterialData = static_cast<MaterialData*>(context->globalMaterialDataBuffer.info.pMappedData);

	// Init default material
	auto defaultMaterial = Material_getDefaultMaterial();
	sceneMaterialData[0] = defaultMaterial.materialData;

	// Write default mesh texture
	defaultMaterial.passType = MaterialPass::MainColor;
	defaultMaterial.material = createMaterialInstance(&context->opaqueMaterial);
	writeUniform(context, &defaultMaterial.material, 1, 0, context->errorCheckerboardImage.imageView, context->defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);

	// Write default material to cache
	MaterialCache_add(context->materialCache, 0, defaultMaterial);

	// Write default viewport texture
	writeUniform(context, &context->uiViewportMaterialInstance, 1, 0, context->errorCheckerboardImage.imageView, context->defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);

	context->mainDeletionQueue.push([context] {
		destroyBuffer(context->vmaAllocator, context->globalMaterialDataBuffer);

		vkDestroySampler(context->device, context->defaultSamplerNearest, nullptr);
		vkDestroySampler(context->device, context->defaultSamplerLinear, nullptr);

		destroyImage(context->device, context->vmaAllocator, context->whiteImage);
		destroyImage(context->device, context->vmaAllocator, context->greyImage);
		destroyImage(context->device, context->vmaAllocator, context->blackImage);
		destroyImage(context->device, context->vmaAllocator, context->errorCheckerboardImage);
	});
}

void initVulkan(VulkanRendererContext* context) {
	vkb::InstanceBuilder builder;

	// make the vulkan instance, with basic debug features
	auto inst = builder.set_app_name("Primal Engine")
								.request_validation_layers(USE_VALIDATION)
								.use_default_debug_messenger()
								.require_api_version(1, 3, 0)
								.build();

	auto vkbInstance = inst.value();

	// grab the instance
	context->instance = vkbInstance.instance;
	context->debugMessenger = vkbInstance.debug_messenger;

	// vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features13{};
	features13.dynamicRendering = VK_TRUE;
	features13.synchronization2 = VK_TRUE;
	features13.maintenance4 = VK_TRUE;

	// vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{};
	features12.bufferDeviceAddress = VK_TRUE;
	features12.descriptorIndexing = VK_TRUE;
	features12.descriptorBindingPartiallyBound = VK_TRUE;
	features12.runtimeDescriptorArray = VK_TRUE;
	features12.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
	features12.descriptorBindingVariableDescriptorCount = VK_TRUE;
	features12.uniformBufferStandardLayout = VK_TRUE;
	features12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;// nonuniformEXT on fragment shader

	VkPhysicalDeviceVulkan11Features features11{};
	features11.shaderDrawParameters = VK_TRUE;

	VkPhysicalDeviceFeatures features{};
	features.textureCompressionBC = VK_TRUE;
	features.multiDrawIndirect = VK_TRUE;
	features.drawIndirectFirstInstance = VK_TRUE;
	features.sampleRateShading = VK_TRUE;
	features.samplerAnisotropy = VK_TRUE;
	features.pipelineStatisticsQuery = VK_TRUE;
	features.fillModeNonSolid = VK_TRUE;

	// Use VKBootstrap to select a gpu.
	// We want a gpu that can write to the surface and supports vulkan 1.3 with the correct features
	vkb::PhysicalDeviceSelector selector{ vkbInstance };
	vkb::PhysicalDevice physicalDevice = selector
																				 .set_minimum_version(1, 3)
																				 .set_required_features(features)
																				 .set_required_features_11(features11)
																				 .set_required_features_13(features13)
																				 .set_required_features_12(features12)
																				 // .set_surface(m_rendererState->window->surface)
																				 .defer_surface_initialization()// NOTE(piero): should we create a "dummy" surface?
																				 .add_required_extension("VK_EXT_scalar_block_layout")
																				 .select()
																				 .value();

	std::cout << std::format("GPU: {}\n", physicalDevice.name);

	// create the final vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a vulkan application
	context->device = vkbDevice.device;
	context->physicalDevice = physicalDevice.physical_device;

	// Make sure we can timestamp and get update period
	assert(physicalDevice.properties.limits.timestampComputeAndGraphics);
	context->physicalDeviceTimestampPeriod = physicalDevice.properties.limits.timestampPeriod;

	context->anisotropyEnabled = physicalDevice.features.samplerAnisotropy;
	context->maxSamplerAnisotropy = physicalDevice.properties.limits.maxSamplerAnisotropy;

	// Check supported native GPU formats for KTX2
	auto formatSupported = [&](VkFormat format) {
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice.physical_device, format, &formatProperties);
		return ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) && (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT));
	};

	// Block compression
	if (physicalDevice.features.textureCompressionBC) {
		if (formatSupported(VK_FORMAT_BC7_SRGB_BLOCK)) {
			context->availableTargetFormats.emplace_back(KTX_TTF_BC7_RGBA);
			context->availableTargetFormatsNames.emplace_back("KTX_TTF_BC7_RGBA");
		}

		if (formatSupported(VK_FORMAT_BC3_SRGB_BLOCK)) {
			context->availableTargetFormats.emplace_back(KTX_TTF_BC3_RGBA);
			context->availableTargetFormatsNames.emplace_back("KTX_TTF_BC3_RGBA");
		}
	}

	// Adaptive scalable texture compression
	if (physicalDevice.features.textureCompressionASTC_LDR) {
		if (formatSupported(VK_FORMAT_ASTC_4x4_SRGB_BLOCK)) {
			context->availableTargetFormats.emplace_back(KTX_TTF_ASTC_4x4_RGBA);
			context->availableTargetFormatsNames.emplace_back("KTX_TTF_ASTC_4x4_RGBA");
		}
	}

	// Ericsson texture compression
	if (physicalDevice.features.textureCompressionETC2) {
		if (formatSupported(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)) {
			context->availableTargetFormats.emplace_back(KTX_TTF_ETC2_RGBA);
			context->availableTargetFormatsNames.emplace_back("KTX_TTF_ETC2_RGBA");
		}
	}

	// Get graphics queue with VKBootstrap
	context->graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	context->graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	// Create allocator using VMA
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = context->physicalDevice;
	allocatorInfo.device = context->device;
	allocatorInfo.instance = context->instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &context->vmaAllocator);
}

void initRenderTargets(VulkanRendererContext* context) {
	VkExtent3D drawImageExtent{
		static_cast<uint32_t>(context->rendererState->window->width),
		static_cast<uint32_t>(context->rendererState->window->height),
		1
	};

	// TODO(piero): what size should this be?
	VkExtent3D sceneDrawImageExtent{
		context->fullScreen ? static_cast<uint32_t>(context->rendererState->window->width) : 1448,
		context->fullScreen ? static_cast<uint32_t>(context->rendererState->window->height - 80) : 700,
		1
	};

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


	auto testWindow = &PrimalEngine::get().windows[1];
	VkExtent3D testWindowExtent{
		static_cast<uint32_t>(testWindow->width),
		static_cast<uint32_t>(testWindow->height),
		1
	};

	// Create render targets
	// TODO(piero): Refactor this. Create a system that does this automatically based on a config file?
	context->rendererState->window->renderTarget = createImage("drawImage", drawImageExtent, context->device, context->vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, drawImageUsages, false);
	testWindow->renderTarget = createImage("scene window drawImage", testWindowExtent, context->device, context->vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, drawImageUsages, false);

	context->sceneDrawImage = createImage("scene drawImage", sceneDrawImageExtent, context->device, context->vmaAllocator, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, false);

	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	context->rendererState->window->depthTarget = createImage("depthImage", drawImageExtent, context->device, context->vmaAllocator, VK_FORMAT_D32_SFLOAT, depthImageUsages, false);
	testWindow->depthTarget = createImage("scene window depthImage", testWindowExtent, context->device, context->vmaAllocator, VK_FORMAT_D32_SFLOAT, depthImageUsages, false);

	context->sceneDepthImage = createImage("scene depthImage", sceneDrawImageExtent, context->device, context->vmaAllocator, VK_FORMAT_D32_SFLOAT, depthImageUsages, false);
}

void initCommands(VulkanRendererContext* context) {
	auto commandPoolInfo = commandPoolCreateInfo(context->graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (auto& frame : context->frames) {
		VK_CHECK(vkCreateCommandPool(context->device, &commandPoolInfo, nullptr, &frame.commandPool));
		auto commandAllocateInfo = commandBufferAllocateInfo(frame.commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(context->device, &commandAllocateInfo, &frame.commandBuffer));
		context->mainDeletionQueue.push([frame, context]() { vkDestroyCommandPool(context->device, frame.commandPool, nullptr); });
	}

	// Create command buffer for immediate submits
	VK_CHECK(vkCreateCommandPool(context->device, &commandPoolInfo, nullptr, &context->immCommandPool));
	VkCommandBufferAllocateInfo cmdAllocInfo = commandBufferAllocateInfo(context->immCommandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(context->device, &cmdAllocInfo, &context->immCommandBuffer));
	context->mainDeletionQueue.push([context]() { vkDestroyCommandPool(context->device, context->immCommandPool, nullptr); });
}

void initSyncStructures(VulkanRendererContext* context) {
	auto fenceCreate = fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);


	VK_CHECK(vkCreateFence(context->device, &fenceCreate, nullptr, &context->immFence));
	context->mainDeletionQueue.push([context]() { vkDestroyFence(context->device, context->immFence, nullptr); });

	// one fence to control when the gpu has finished rendering the frame,
	// we want the fence to start signaled so we can wait on it on the first frame
	for (auto& frame : context->frames) {
		VK_CHECK(vkCreateFence(context->device, &fenceCreate, nullptr, &frame.renderFence));

		context->mainDeletionQueue.push([frame, context] {
			vkDestroyFence(context->device, frame.renderFence, nullptr);
		});
	}
}

void rendererCleanup(VulkanRendererContext* context) {
	vkDeviceWaitIdle(context->device);

	for (auto& model : context->loadedModels) {
		cleanupModel(context, model);
	}

	context->loadedModels.clear();

	for (auto& frame : context->frames) {
		frame.deletionQueue.flush();
	}

	// destroy sceneDrawImage
	destroyImage(context->device, context->vmaAllocator, context->sceneDrawImage);
	destroyImage(context->device, context->vmaAllocator, context->sceneDepthImage);

	cleanupTerrain(context);

	// destroyGBuffer(context);

	destroyImage(context->device, context->vmaAllocator, context->sourceCodeFontTexture);

	destroyFontSDF(context->sourceCodeFont);
	// destroyFontSDF(context->arialFont);

	// destroyImage(context->device, context->vmaAllocator, context->blueNoise);

	vkDestroyPipelineCache(context->device, context->pipelineCache, nullptr);

	context->mainDeletionQueue.flush();

	UI::cleanupRenderContext();

	for (auto& window : PrimalEngine::get().windows) {
		destroyPrimalWindow(&window, context->vmaAllocator, context->device, context->instance, nullptr);
	}

	vmaDestroyAllocator(context->vmaAllocator);
	vkDestroyDevice(context->device, nullptr);
	vkb::destroy_debug_utils_messenger(context->instance, context->debugMessenger);
	vkDestroyInstance(context->instance, nullptr);
}

std::vector<UI::UIElement> buildUIGeometry(VulkanRendererContext* context, FixedArray<UI::UIRenderCommand>& renderCommands, std::vector<UI::UIVertex>& vertices, std::vector<uint32_t>& indices) {
	std::vector<UI::UIElement> elements;

	for (uint32_t i = 0; i < renderCommands.length; ++i) {
		auto renderCommand = FixedArray_get(renderCommands, i);
		switch (renderCommand->commandType) {
		case UI::UIRenderCommandType::RECTANGLE: {
			elements.push_back(UI::box(vertices, indices));
			break;
		}
		case UI::UIRenderCommandType::TITLEBAR: {
			elements.push_back(UI::box(vertices, indices));
			break;
		}
		case UI::UIRenderCommandType::VIEWPORT: {
			elements.push_back(UI::box(vertices, indices));
			break;
		}
		case UI::UIRenderCommandType::PANEL: {
			elements.push_back(UI::box(vertices, indices));
			break;
		}
		case UI::UIRenderCommandType::DOCKSPACE: {
			elements.push_back(UI::box(vertices, indices));
			break;
		}
		case UI::UIRenderCommandType::CIRCLE: {
			if (renderCommand->circleType == UI::CircleType::FILLED) {
				elements.push_back(UI::circleFilled(renderCommand->radius, renderCommand->segments, vertices, indices));
			} else {
				elements.push_back(UI::circle(renderCommand->radius, renderCommand->segments, renderCommand->thickness, vertices, indices));
			}
			break;
		}
		case pm::UI::UIRenderCommandType::TEXT: {
			elements.push_back(UI::text(renderCommand->text, 16.0f, &context->sourceCodeFont, &vertices, &indices));
			break;
		}
		}
	}

	return elements;
}

void buildUIDrawBatches(VulkanRendererContext* context, FixedArray<UI::UIWindowBatchCommands>& windowBatches) {
	auto start = std::chrono::system_clock::now();

	getCurrentFrame(context).uiWindowBatches.clear();

	for (uint32_t i = 0; i < windowBatches.length; ++i) {

		auto batch = FixedArray_get(windowBatches, i);
		auto& renderCommands = batch->renderCommands;

		UIWindowBatch windowBatch{
			.window = batch->window
		};
		windowBatch.drawBatches.reserve(renderCommands.length);

		std::vector<UIDrawData> uiDrawData;
		std::vector<glm::mat4> textTransformData;
		std::vector<ViewportDrawData> viewportDrawData;

		std::vector<UIIndirectCommand> uiDrawCommands;
		std::vector<UIIndirectCommand> textDrawCommands;
		std::vector<UIIndirectCommand> viewportDrawCommands;

		std::vector<uint32_t> viewportImageViewIds;

		DrawBatch uiDrawBatch{ .type = DrawBatchType::UI_BATCH };
		DrawBatch textDrawBatch{ .type = DrawBatchType::TEXT_BATCH };
		DrawBatch viewportDrawBatch{ .type = DrawBatchType::VIEWPORT_BATCH };

		std::vector<UI::UIVertex> vertices;
		std::vector<uint32_t> indices;

		auto elements = buildUIGeometry(context, renderCommands, vertices, indices);

		// We use one Vertex/index buffer for all UI geometry in each window
		auto uiGeometryBuffers = uploadMesh(context, indices, vertices, std::format("uiMeshBuffers-{}", batch->window->id));

		std::vector<UIMaterialData> uiMaterialData;

		uiDrawBatch.meshBuffers = uiGeometryBuffers;
		uiDrawBatch.pipeline = context->uiMaterial.pipeline;
		uiDrawBatch.pipelineLayout = context->uiMaterial.pipelineLayout;

		textDrawBatch.meshBuffers = uiGeometryBuffers;
		textDrawBatch.pipeline = context->uiTextMaterial.pipeline;
		textDrawBatch.pipelineLayout = context->uiTextMaterial.pipelineLayout;

		viewportDrawBatch.meshBuffers = uiGeometryBuffers;
		viewportDrawBatch.pipeline = context->uiViewportMaterial.pipeline;
		viewportDrawBatch.pipelineLayout = context->uiViewportMaterial.pipelineLayout;

		uint32_t uiDrawIdCount{ 0 }, textDrawIdCount{ 0 }, viewportDrawIdCount{ 0 };

		for (uint32_t i = 0; i < elements.size(); ++i) {
			auto renderCommand = FixedArray_get(renderCommands, i);
			auto& uiElement = elements.at(i);

			auto& rect = renderCommand->boundingRect;
			auto transform = glm::mat4{ 1.0f };

			switch (renderCommand->commandType) {
			case UI::UIRenderCommandType::RECTANGLE: {
				transform = glm::translate(transform, glm::vec3(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f, 0.0f));
				// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, glm::vec3(rect.width / 2.0f, rect.height / 2.0f, 1.0f));
				uiDrawCommands.push_back({ .drawId = uiDrawIdCount,
					.command = {
						.indexCount = uiElement.indexCount,
						.instanceCount = 1,
						.firstIndex = uiElement.firstIndex,
						.vertexOffset = uiElement.vertexOffset,
						.firstInstance = uiDrawIdCount } });
				uiDrawData.push_back({ .transform = transform, .materialIndex = uiDrawIdCount });
				uiMaterialData.push_back({ .backgroundColor = renderCommand->backgroundColor, .horizontalBorder = renderCommand->border.horizontalBorder, .verticalBorder = renderCommand->border.verticalBorder });
				uiDrawIdCount++;
				break;
			}
			case UI::UIRenderCommandType::VIEWPORT: {
				transform = glm::translate(transform, glm::vec3(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f, 0.0f));
				// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, glm::vec3(rect.width / 2.0f, rect.height / 2.0f, 1.0f));
				viewportDrawCommands.push_back({ .drawId = viewportDrawIdCount,
					.command = {
						.indexCount = uiElement.indexCount,
						.instanceCount = 1,
						.firstIndex = uiElement.firstIndex,
						.vertexOffset = uiElement.vertexOffset,
						.firstInstance = viewportDrawIdCount } });
				viewportDrawData.push_back({ .transform = transform, .textureIndex = renderCommand->textureId });
				viewportDrawIdCount++;
				break;
			}
			case UI::UIRenderCommandType::PANEL: {
				transform = glm::translate(transform, glm::vec3(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f, 0.0f));
				// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, glm::vec3(rect.width / 2.0f, rect.height / 2.0f, 1.0f));
				uiDrawCommands.push_back({ .drawId = uiDrawIdCount,
					.command = {
						.indexCount = uiElement.indexCount,
						.instanceCount = 1,
						.firstIndex = uiElement.firstIndex,
						.vertexOffset = uiElement.vertexOffset,
						.firstInstance = uiDrawIdCount } });
				uiDrawData.push_back({ .transform = transform, .materialIndex = uiDrawIdCount });
				uiMaterialData.push_back({ .backgroundColor = renderCommand->backgroundColor, .horizontalBorder = renderCommand->border.horizontalBorder, .verticalBorder = renderCommand->border.verticalBorder });
				uiDrawIdCount++;
				break;
			}
			case UI::UIRenderCommandType::TITLEBAR: {
				transform = glm::translate(transform, glm::vec3(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f, 0.0f));
				// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, glm::vec3(rect.width / 2.0f, rect.height / 2.0f, 1.0f));
				uiDrawCommands.push_back({ .drawId = uiDrawIdCount,
					.command = {
						.indexCount = uiElement.indexCount,
						.instanceCount = 1,
						.firstIndex = uiElement.firstIndex,
						.vertexOffset = uiElement.vertexOffset,
						.firstInstance = uiDrawIdCount } });
				uiDrawData.push_back({ .transform = transform, .materialIndex = uiDrawIdCount });
				uiMaterialData.push_back({ .backgroundColor = renderCommand->backgroundColor, .horizontalBorder = renderCommand->border.horizontalBorder, .verticalBorder = renderCommand->border.verticalBorder });
				uiDrawIdCount++;
				break;
			}
			case UI::UIRenderCommandType::DOCKSPACE: {
				transform = glm::translate(transform, glm::vec3(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f, 0.0f));
				// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, glm::vec3(rect.width / 2.0f, rect.height / 2.0f, 1.0f));
				uiDrawCommands.push_back({ .drawId = uiDrawIdCount,
					.command = {
						.indexCount = uiElement.indexCount,
						.instanceCount = 1,
						.firstIndex = uiElement.firstIndex,
						.vertexOffset = uiElement.vertexOffset,
						.firstInstance = uiDrawIdCount } });
				uiDrawData.push_back({ .transform = transform, .materialIndex = uiDrawIdCount });
				uiMaterialData.push_back({ .backgroundColor = renderCommand->backgroundColor, .horizontalBorder = renderCommand->border.horizontalBorder, .verticalBorder = renderCommand->border.verticalBorder });
				uiDrawIdCount++;
				break;
			}
			case UI::UIRenderCommandType::CIRCLE: {
				transform = glm::translate(transform, glm::vec3(rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f, 0.0f));
				// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));
				uiDrawCommands.push_back({ .drawId = uiDrawIdCount,
					.command = {
						.indexCount = uiElement.indexCount,
						.instanceCount = 1,
						.firstIndex = uiElement.firstIndex,
						.vertexOffset = uiElement.vertexOffset,
						.firstInstance = uiDrawIdCount } });
				uiDrawData.push_back({ .transform = transform, .materialIndex = uiDrawIdCount });
				uiMaterialData.push_back({ .backgroundColor = renderCommand->backgroundColor });
				uiDrawIdCount++;
				break;
			}
			case UI::UIRenderCommandType::TEXT: {
				transform = glm::translate(transform, glm::vec3(rect.x, rect.y, 0.0f));
				// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
				transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));
				textDrawCommands.push_back({ .drawId = textDrawIdCount,
					.command = {
						.indexCount = uiElement.indexCount,
						.instanceCount = 1,
						.firstIndex = uiElement.firstIndex,
						.vertexOffset = uiElement.vertexOffset,
						.firstInstance = textDrawIdCount } });

				textTransformData.push_back(transform);
				textDrawIdCount++;
				break;
			}
			default: {
				std::cout << "Command type not implemented, skipping.\n";
				break;
			}
			}
		}

		auto uiDrawCommandsBuffer = createBuffer("uiIndirectCommandBuffer", sizeof(UIIndirectCommand) * uiDrawCommands.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(uiDrawCommandsBuffer.info.pMappedData, uiDrawCommands.data(), sizeof(UIIndirectCommand) * uiDrawCommands.size());

		auto uiDrawDataBuffer = createBuffer("uiDrawDataBuffer", sizeof(UIDrawData) * uiDrawData.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(uiDrawDataBuffer.info.pMappedData, uiDrawData.data(), sizeof(UIDrawData) * uiDrawData.size());

		auto uiMaterialDataBuffer = createBuffer("uiMaterialDataBuffer", sizeof(UIMaterialData) * uiMaterialData.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(uiMaterialDataBuffer.info.pMappedData, uiMaterialData.data(), sizeof(UIMaterialData) * uiMaterialData.size());

		uiDrawBatch.commands = {
			.buffer = uiDrawCommandsBuffer,
			.offset = offsetof(UIIndirectCommand, command),
			.size = static_cast<uint32_t>(uiDrawCommands.size()),
			.stride = sizeof(UIIndirectCommand)
		};

		uiDrawBatch.material = createMaterialInstance(&context->uiMaterial);

		uiDrawBatch.material.descriptorSets.at(0) = getCurrentFrame(context).frameDescriptor.allocate(context->device, uiDrawBatch.material.material->descriptorLayouts.at(0));
		writeUniform(context, &uiDrawBatch.material, 0, 0, batch->window->uiData.buffer, sizeof(UIUniformData), 0);
		writeUniform(context, &uiDrawBatch.material, 0, 1, uiDrawCommandsBuffer.buffer, sizeof(UIIndirectCommand) * uiDrawCommands.size(), 0);
		writeUniform(context, &uiDrawBatch.material, 0, 2, uiDrawDataBuffer.buffer, sizeof(UIDrawData) * uiDrawData.size(), 0);
		writeUniform(context, &uiDrawBatch.material, 0, 3, uiMaterialDataBuffer.buffer, sizeof(UIMaterialData) * uiMaterialData.size(), 0);

		windowBatch.drawBatches.push_back(uiDrawBatch);

		// text
		if (textDrawCommands.size() > 0) {
			auto textDrawCommandsBuffer = createBuffer("textIndirectCommandBuffer", sizeof(UIIndirectCommand) * textDrawCommands.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			memcpy(textDrawCommandsBuffer.info.pMappedData, textDrawCommands.data(), sizeof(UIIndirectCommand) * textDrawCommands.size());

			auto textTransformDataBuffer = createBuffer("textTransformBuffer", sizeof(glm::mat4) * textTransformData.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			memcpy(textTransformDataBuffer.info.pMappedData, textTransformData.data(), sizeof(glm::mat4) * textTransformData.size());

			textDrawBatch.commands = {
				.buffer = textDrawCommandsBuffer,
				.offset = offsetof(UIIndirectCommand, command),
				.size = static_cast<uint32_t>(textDrawCommands.size()),
				.stride = sizeof(UIIndirectCommand)
			};

			textDrawBatch.material = createMaterialInstance(&context->uiTextMaterial);

			// NOTE(piero): Allocate a frame descriptor set and write uniforms for this material.
			textDrawBatch.material.descriptorSets.at(0) = getCurrentFrame(context).frameDescriptor.allocate(context->device, textDrawBatch.material.material->descriptorLayouts.at(0));
			writeUniform(context, &textDrawBatch.material, 0, 0, batch->window->fontData.buffer, sizeof(FontUniformData), 0);
			writeUniform(context, &textDrawBatch.material, 0, 1, context->sourceCodeFontTexture.imageView, context->sourceCodeFontTexture.sampler, context->sourceCodeFontTexture.imageLayout);
			writeUniform(context, &textDrawBatch.material, 0, 2, textDrawCommandsBuffer.buffer, sizeof(UIIndirectCommand) * textDrawCommands.size(), 0);
			writeUniform(context, &textDrawBatch.material, 0, 3, textTransformDataBuffer.buffer, sizeof(glm::mat4) * textTransformData.size(), 0);


			windowBatch.drawBatches.push_back(textDrawBatch);

			getCurrentFrame(context).deletionQueue.push([context, textDrawCommandsBuffer, textTransformDataBuffer]() {
				destroyBuffer(context->vmaAllocator, textDrawCommandsBuffer);
				destroyBuffer(context->vmaAllocator, textTransformDataBuffer);
			});
		}

		// viewport
		if (viewportDrawCommands.size() > 0) {
			auto viewportDrawCommandsBuffer = createBuffer("viewportIndirectCommandBuffer", sizeof(UIIndirectCommand) * viewportDrawCommands.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			memcpy(viewportDrawCommandsBuffer.info.pMappedData, viewportDrawCommands.data(), sizeof(UIIndirectCommand) * viewportDrawCommands.size());

			auto viewportTransformDataBuffer = createBuffer("viewportTransformBuffer", sizeof(ViewportDrawData) * viewportDrawData.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			memcpy(viewportTransformDataBuffer.info.pMappedData, viewportDrawData.data(), sizeof(ViewportDrawData) * viewportDrawData.size());

			viewportDrawBatch.commands = {
				.buffer = viewportDrawCommandsBuffer,
				.offset = offsetof(UIIndirectCommand, command),
				.size = static_cast<uint32_t>(viewportDrawCommands.size()),
				.stride = sizeof(UIIndirectCommand)
			};

			viewportDrawBatch.material = context->uiViewportMaterialInstance;

			viewportDrawBatch.material.descriptorSets.at(0) = getCurrentFrame(context).frameDescriptor.allocate(context->device, viewportDrawBatch.material.material->descriptorLayouts.at(0));
			writeUniform(context, &viewportDrawBatch.material, 0, 0, batch->window->uiData.buffer, sizeof(UIUniformData), 0);
			writeUniform(context, &viewportDrawBatch.material, 0, 1, viewportDrawCommandsBuffer.buffer, sizeof(UIIndirectCommand) * viewportDrawCommands.size(), 0);
			writeUniform(context, &viewportDrawBatch.material, 0, 2, viewportTransformDataBuffer.buffer, sizeof(ViewportDrawData) * viewportDrawData.size(), 0);

			windowBatch.drawBatches.push_back(viewportDrawBatch);

			getCurrentFrame(context).deletionQueue.push([context, viewportDrawCommandsBuffer, viewportTransformDataBuffer]() {
				destroyBuffer(context->vmaAllocator, viewportDrawCommandsBuffer);
				destroyBuffer(context->vmaAllocator, viewportTransformDataBuffer);
			});
		}

		getCurrentFrame(context).deletionQueue.push([context, uiDrawCommandsBuffer, uiDrawDataBuffer, uiMaterialDataBuffer, uiGeometryBuffers]() {
			destroyBuffer(context->vmaAllocator, uiDrawCommandsBuffer);
			destroyBuffer(context->vmaAllocator, uiDrawDataBuffer);
			destroyBuffer(context->vmaAllocator, uiMaterialDataBuffer);
			destroyBuffer(context->vmaAllocator, uiGeometryBuffers.vertexBuffer);
			destroyBuffer(context->vmaAllocator, uiGeometryBuffers.indexBuffer);
		});

		getCurrentFrame(context).uiWindowBatches.push_back(windowBatch);
	}

	auto genTime = std::chrono::duration<double, std::micro>(std::chrono::system_clock::now() - start).count();

	context->rendererState->rendererStats.uiDrawBatchGenerationTimeAvg = context->rendererState->rendererStats.uiDrawBatchGenerationTimeAvg * 0.95 + genTime * 0.05;
}

void buildDrawBatches(VulkanRendererContext* context, std::vector<Model>& models) {
	getCurrentFrame(context).drawBatches.clear();

	// TODO: Right now each model uses their own vertex and index buffers
	// 			 This might not be efficient since we can, very likely,
	// 			 combine multiple models and render them in the same draw batches
	// 			 if they use the same vertex/index buffers.
	double flattenTime{};
	double genTime{};
	for (auto model : models) {
		auto start = std::chrono::high_resolution_clock::now();

		DrawBatch opaque{ .type = DrawBatchType::MESH_BATCH };
		opaque.meshBuffers = model.modelBuffers;
		opaque.material = createMaterialInstance(&context->opaqueMaterial);

		DrawBatch transparent{ .type = DrawBatchType::MESH_BATCH };
		transparent.meshBuffers = model.modelBuffers;
		transparent.material = createMaterialInstance(&context->transparentMaterial);

		DrawBatch doubleSided{ .type = DrawBatchType::MESH_BATCH };
		doubleSided.meshBuffers = model.modelBuffers;
		doubleSided.material = createMaterialInstance(&context->doubleSidedMaterial);

		auto opaqueCommands = std::vector<MeshIndirectCommand>();
		auto doubleSidedCommands = std::vector<MeshIndirectCommand>();
		auto transparentCommands = std::vector<MeshIndirectCommand>();

		std::vector<MeshDraw> opaqueDraws{};
		std::vector<MeshDraw> doubleSidedDraws{};
		std::vector<MeshDraw> transparentDraws{};

		// NOTE(piero): generate a draw command for each entity with a mesh from this model
		std::queue<Entity*> q;
		q.push(model.root);

		while (!q.empty()) {
			auto& entity = q.front();
			q.pop();

			// NOTE(piero): We are not refreshing transforms right now since all entities are static.
			//              Once we start having dynamic entities, we should implement some "dirty" state to refresh transforms.

			for (auto c : entity->children) {
				q.push(c);
			}

			if (entity->mesh == nullptr) {
				continue;
			}

			auto s = std::chrono::high_resolution_clock::now();
			for (auto& primitive : entity->mesh->primitives) {
				if (primitive.passType == MaterialPass::Transparent) {
					auto drawId = static_cast<uint32_t>(transparentCommands.size());
					transparentCommands.push_back({ .drawId = drawId,
						.command = {
							.indexCount = primitive.indexCount,
							.instanceCount = 1,
							.firstIndex = primitive.firstIndex,
							.vertexOffset = primitive.vertexOffset,
							.firstInstance = drawId } });
					transparentDraws.push_back({ .transform = entity->worldTransform, .materialIndex = primitive.materialIndex });
				} else if (primitive.passType == MaterialPass::DoubleSided) {
					auto drawId = static_cast<uint32_t>(doubleSidedCommands.size());
					doubleSidedCommands.push_back({ .drawId = drawId,
						.command = {
							.indexCount = primitive.indexCount,
							.instanceCount = 1,
							.firstIndex = primitive.firstIndex,
							.vertexOffset = primitive.vertexOffset,
							.firstInstance = drawId } });
					doubleSidedDraws.push_back({ .transform = entity->worldTransform, .materialIndex = primitive.materialIndex });
				} else {
					auto drawId = static_cast<uint32_t>(opaqueCommands.size());
					opaqueCommands.push_back({ .drawId = drawId,
						.command = {
							.indexCount = primitive.indexCount,
							.instanceCount = 1,
							.firstIndex = primitive.firstIndex,
							.vertexOffset = primitive.vertexOffset,
							.firstInstance = drawId } });
					opaqueDraws.push_back({ .transform = entity->worldTransform, .materialIndex = primitive.materialIndex });
				}
			}
			flattenTime += std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - s).count();
		}

		auto meshIndirectDoubleSidedCommandsBuffer = createBuffer("meshDrawCommandsBuffer DoubleSided", sizeof(MeshIndirectCommand) * doubleSidedCommands.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(meshIndirectDoubleSidedCommandsBuffer.info.pMappedData, doubleSidedCommands.data(), sizeof(MeshIndirectCommand) * doubleSidedCommands.size());

		auto meshIndirectTransparentCommandsBuffer = createBuffer("meshDrawCommandsBuffer Transparent", sizeof(MeshIndirectCommand) * transparentCommands.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(meshIndirectTransparentCommandsBuffer.info.pMappedData, transparentCommands.data(), sizeof(MeshIndirectCommand) * transparentCommands.size());

		auto doubleSidedDrawsDataBuffer = createBuffer("meshTransformBuffer Opaque", sizeof(MeshDraw) * doubleSidedDraws.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(doubleSidedDrawsDataBuffer.info.pMappedData, doubleSidedDraws.data(), sizeof(MeshDraw) * doubleSidedDraws.size());

		auto transparentDrawsDataBuffer = createBuffer("meshTransformBuffer Transparent", sizeof(MeshDraw) * transparentDraws.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(transparentDrawsDataBuffer.info.pMappedData, transparentDraws.data(), sizeof(MeshDraw) * transparentDraws.size());

		// Create buffer for global scene data
		AllocatedBuffer gpuSceneDataBuffer = createBuffer("gpuSceneDataBuffer", sizeof(GPUSceneData), context->vmaAllocator, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		memcpy(gpuSceneDataBuffer.info.pMappedData, &context->sceneData, sizeof(GPUSceneData));

		getCurrentFrame(context).deletionQueue.push([gpuSceneDataBuffer, context]() {
			destroyBuffer(context->vmaAllocator, gpuSceneDataBuffer);
		});

		doubleSided.material.descriptorSets.at(0) = getCurrentFrame(context).frameDescriptor.allocate(context->device, doubleSided.material.material->descriptorLayouts.at(0));
		writeUniform(context, &doubleSided.material, 0, 0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0);

		doubleSided.material.descriptorSets.at(2) = getCurrentFrame(context).frameDescriptor.allocate(context->device, doubleSided.material.material->descriptorLayouts.at(2));
		writeUniform(context, &doubleSided.material, 2, 0, context->globalMaterialDataBuffer.buffer, sizeof(MaterialData) * MaterialCache_size(context->materialCache), 0);
		writeUniform(context, &doubleSided.material, 2, 1, meshIndirectDoubleSidedCommandsBuffer.buffer, sizeof(MeshIndirectCommand) * doubleSidedCommands.size(), 0);
		writeUniform(context, &doubleSided.material, 2, 2, doubleSidedDrawsDataBuffer.buffer, sizeof(MeshDraw) * doubleSidedDraws.size(), 0);

		transparent.material.descriptorSets.at(0) = getCurrentFrame(context).frameDescriptor.allocate(context->device, transparent.material.material->descriptorLayouts.at(0));
		writeUniform(context, &transparent.material, 0, 0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0);

		transparent.material.descriptorSets.at(2) = getCurrentFrame(context).frameDescriptor.allocate(context->device, transparent.material.material->descriptorLayouts.at(2));
		writeUniform(context, &transparent.material, 2, 0, context->globalMaterialDataBuffer.buffer, sizeof(MaterialData) * MaterialCache_size(context->materialCache), 0);
		writeUniform(context, &transparent.material, 2, 1, meshIndirectTransparentCommandsBuffer.buffer, sizeof(MeshIndirectCommand) * transparentCommands.size(), 0);
		writeUniform(context, &transparent.material, 2, 2, transparentDrawsDataBuffer.buffer, sizeof(MeshDraw) * transparentDraws.size(), 0);

		transparent.commands = {
			.buffer = meshIndirectTransparentCommandsBuffer,
			.offset = offsetof(MeshIndirectCommand, command),
			.size = static_cast<uint32_t>(transparentCommands.size()),
			.stride = sizeof(MeshIndirectCommand)
		};

		doubleSided.commands = {
			.buffer = meshIndirectDoubleSidedCommandsBuffer,
			.offset = offsetof(MeshIndirectCommand, command),
			.size = static_cast<uint32_t>(doubleSidedCommands.size()),
			.stride = sizeof(MeshIndirectCommand)
		};

		if (opaqueCommands.size() > 0) {
			auto meshIndirectOpaqueCommandsBuffer = createBuffer("meshDrawCommandsBuffer Opaque", sizeof(MeshIndirectCommand) * opaqueCommands.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			memcpy(meshIndirectOpaqueCommandsBuffer.info.pMappedData, opaqueCommands.data(), sizeof(MeshIndirectCommand) * opaqueCommands.size());

			auto opaqueDrawsDataBuffer = createBuffer("meshTransformBuffer Opaque", sizeof(MeshDraw) * opaqueDraws.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			memcpy(opaqueDrawsDataBuffer.info.pMappedData, opaqueDraws.data(), sizeof(MeshDraw) * opaqueDraws.size());

			opaque.material.descriptorSets.at(0) = getCurrentFrame(context).frameDescriptor.allocate(context->device, opaque.material.material->descriptorLayouts.at(0));
			writeUniform(context, &opaque.material, 0, 0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0);

			opaque.material.descriptorSets.at(2) = getCurrentFrame(context).frameDescriptor.allocate(context->device, opaque.material.material->descriptorLayouts.at(2));
			writeUniform(context, &opaque.material, 2, 0, context->globalMaterialDataBuffer.buffer, sizeof(MaterialData) * MaterialCache_size(context->materialCache), 0);
			writeUniform(context, &opaque.material, 2, 1, meshIndirectOpaqueCommandsBuffer.buffer, sizeof(MeshIndirectCommand) * opaqueCommands.size(), 0);
			writeUniform(context, &opaque.material, 2, 2, opaqueDrawsDataBuffer.buffer, sizeof(MeshDraw) * opaqueDraws.size(), 0);

			opaque.commands = {
				.buffer = meshIndirectOpaqueCommandsBuffer,
				.offset = offsetof(MeshIndirectCommand, command),
				.size = static_cast<uint32_t>(opaqueCommands.size()),
				.stride = sizeof(MeshIndirectCommand)
			};

			getCurrentFrame(context).drawBatches.push_back(opaque);

			getCurrentFrame(context).deletionQueue.push([context, opaqueDrawsDataBuffer, meshIndirectOpaqueCommandsBuffer]() {
				destroyBuffer(context->vmaAllocator, opaqueDrawsDataBuffer);
				destroyBuffer(context->vmaAllocator, meshIndirectOpaqueCommandsBuffer);
			});
		}

		getCurrentFrame(context).drawBatches.push_back(doubleSided);
		getCurrentFrame(context).drawBatches.push_back(transparent);

		getCurrentFrame(context).deletionQueue.push([context, doubleSidedDrawsDataBuffer, transparentDrawsDataBuffer, meshIndirectDoubleSidedCommandsBuffer, meshIndirectTransparentCommandsBuffer]() {
			destroyBuffer(context->vmaAllocator, doubleSidedDrawsDataBuffer);
			destroyBuffer(context->vmaAllocator, transparentDrawsDataBuffer);
			destroyBuffer(context->vmaAllocator, meshIndirectDoubleSidedCommandsBuffer);
			destroyBuffer(context->vmaAllocator, meshIndirectTransparentCommandsBuffer);
		});
		genTime += std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - start).count();
	}

	context->rendererState->rendererStats.entityFlattenTimeAvg = context->rendererState->rendererStats.entityFlattenTimeAvg * 0.95 + flattenTime * 0.05;
	context->rendererState->rendererStats.drawBatchGenerationTimeAvg = context->rendererState->rendererStats.drawBatchGenerationTimeAvg * 0.95 + genTime * 0.05;
}

void rendererUpdate(VulkanRendererContext* context, float deltaTime) {
	// TEMP(piero): frame count for random sequences on GPU
	context->rendererState->rendererStats.frameCount++;

	updateScene(context, deltaTime);
	updateUIData(context);
	updateFontData(context);
}

void rendererDraw(VulkanRendererContext* context) {
	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(context->device, 1, &getCurrentFrame(context).renderFence, true, 1000000000));

	getCurrentFrame(context).deletionQueue.flush();
	getCurrentFrame(context).frameDescriptor.clearPools(context->device);

	// Get next swapchain image for each swapchain/window we render to
	auto currentFrameIndex = getCurrentFrameIndex(context);
	for (auto& window : PrimalEngine::get().windows) {
		auto e = vkAcquireNextImageKHR(context->device, window.swapchain.handle, 1000000000, window.swapchain.swapchainSemaphores.at(currentFrameIndex), nullptr, &window.nextImageIndex);
		if (e == VK_ERROR_OUT_OF_DATE_KHR) {
			window.resizeRequested = true;
			return;
		}
	}

	VK_CHECK(vkResetFences(context->device, 1, &getCurrentFrame(context).renderFence));

	// Build draw batches
	buildDrawBatches(context, context->loadedModels);
	buildUIDrawBatches(context, getCurrentFrame(context).uiWindowBatchCommands);

	auto commandBuffer = getCurrentFrame(context).commandBuffer;

	VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

	auto commandBeginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBeginInfo));

	vkCmdResetQueryPool(commandBuffer, context->timestampPool, 0, 128);
	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, context->timestampPool, 0);

	// transition GBuffer targets so compute can write to them
	/*
	transitionImage(commandBuffer, context->gbuffer.albedo.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	transitionImage(commandBuffer, context->gbuffer.irradiance.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	transitionImage(commandBuffer, context->gbuffer.depth.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// TEMP: clear depth buffer
	VkClearColorValue clearValue = {};
	clearValue.float32[0] = 0.0f;
	clearValue.float32[1] = 0.0f;
	clearValue.float32[2] = 0.0f;
	clearValue.float32[3] = 0.0f;

	VkImageSubresourceRange range = {};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;

	vkCmdClearColorImage(
		commandBuffer,
		context->gbuffer.depth.image,
		VK_IMAGE_LAYOUT_GENERAL,
		&clearValue,
		1,
		&range);

	drawToGBuffer(context, commandBuffer);
	*/

	// transition render targets into correct layouts
	for (auto& window : PrimalEngine::get().windows) {
		transitionImage(commandBuffer, window.renderTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		transitionImage(commandBuffer, window.depthTarget.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	}

	transitionImage(commandBuffer, context->sceneDrawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	transitionImage(commandBuffer, context->sceneDepthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	vkCmdResetQueryPool(commandBuffer, context->pipelineStatisticsPool, 0, 1);
	vkCmdBeginQuery(commandBuffer, context->pipelineStatisticsPool, 0, 0);

	// drawGeometry(context, commandBuffer);
	drawTerrain(context, commandBuffer);
	transitionImage(commandBuffer, context->sceneDrawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// blit gbuffer to sceneDrawImage
	/*
	transitionImage(commandBuffer, context->gbuffer.albedo.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	transitionImage(commandBuffer, context->gbuffer.irradiance.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	transitionImage(commandBuffer, context->gbuffer.depth.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	transitionImage(commandBuffer, context->sceneDrawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

	blitGBuffer(context, commandBuffer);

	transitionImage(commandBuffer, context->sceneDrawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	*/

	drawUI(context, commandBuffer);

	vkCmdEndQuery(commandBuffer, context->pipelineStatisticsPool, 0);

	// transition the render targets into their correct transfer layouts
	for (auto& window : PrimalEngine::get().windows) {
		transitionImage(commandBuffer, window.renderTarget.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}

	// Copy render target to swapchain image
	for (auto& window : PrimalEngine::get().windows) {
		// get all swapchains ready to be copied to
		transitionImage(commandBuffer, window.swapchain.images[window.nextImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		// execute a copy from the draw image into the swapchain
		VkExtent2D sourceImageSize{ .width = window.renderTarget.imageExtent.width, .height = window.renderTarget.imageExtent.height };
		copyImageToImage(commandBuffer, window.renderTarget.image, window.swapchain.images[window.nextImageIndex], sourceImageSize, window.swapchain.extent);

		// set swapchain image layout to Present so we can show it on the screen
		transitionImage(commandBuffer, window.swapchain.images[window.nextImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, context->timestampPool, 1);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	// prepare the submission to the queue.
	// we want to wait on the swapchainSemaphore, as that semaphore is signaled when the swapchain is ready
	// we will signal the renderSemaphore, to signal that rendering has finished
	auto cmdinfo = commandBufferSubmitInfo(commandBuffer);

	std::vector<VkSemaphoreSubmitInfo> waitInfos{};
	waitInfos.reserve(PrimalEngine::get().windows.size());

	std::vector<VkSemaphoreSubmitInfo> signalInfos{};
	signalInfos.reserve(PrimalEngine::get().windows.size());

	for (auto& window : PrimalEngine::get().windows) {
		waitInfos.push_back(semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, window.swapchain.swapchainSemaphores.at(currentFrameIndex)));
		signalInfos.push_back(semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, window.swapchain.renderSemaphores.at(currentFrameIndex)));
	}

	VkSubmitInfo2 submit = submitInfo(&cmdinfo, &signalInfos, &waitInfos);

	// submit command buffer to the queue and execute it.
	// m_renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(context->graphicsQueue, 1, &submit, getCurrentFrame(context).renderFence));

	// prepare presenting all visible windows
	// we want to wait on the renderSemaphore for that,
	// as its necessary that drawing commands have finished before the image is displayed to the user
	for (auto& window : PrimalEngine::get().windows) {
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pSwapchains = &window.swapchain.handle;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &window.swapchain.renderSemaphores.at(currentFrameIndex);
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &window.nextImageIndex;

		VkResult presentResult = vkQueuePresentKHR(context->graphicsQueue, &presentInfo);
		if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
			window.resizeRequested = true;
		}
	}

	// TODO: we are waiting on fences twice inside this function. Need to rework this logic.
	// This is just here so we can query timestamp results.
	VK_CHECK(vkWaitForFences(context->device, 1, &getCurrentFrame(context).renderFence, VK_TRUE, ~0ull));

	// Get timestamp results
	std::array<uint64_t, 2> timestampResults{};
	VK_CHECK(vkGetQueryPoolResults(context->device, context->timestampPool, 0, timestampResults.size(), timestampResults.size() * sizeof(uint64_t), timestampResults.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT));

	std::array<uint64_t, 1> pipelineStatisticsResults{};
	VK_CHECK(vkGetQueryPoolResults(context->device, context->pipelineStatisticsPool, 0, pipelineStatisticsResults.size(), pipelineStatisticsResults.size() * sizeof(uint64_t), pipelineStatisticsResults.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT));

	double frameGpuBegin = double(timestampResults[0]) * context->physicalDeviceTimestampPeriod * 1e-6;
	double frameGpuEnd = double(timestampResults[1]) * context->physicalDeviceTimestampPeriod * 1e-6;
	context->rendererState->rendererStats.frameGpuTimeAvg = context->rendererState->rendererStats.frameGpuTimeAvg * 0.95 + (frameGpuEnd - frameGpuBegin) * 0.05;
	context->rendererState->rendererStats.triangleCount = pipelineStatisticsResults[0];

	// move to the next frame.
	context->frameNumber++;
}

void drawToGBuffer(VulkanRendererContext* context, VkCommandBuffer commandBuffer) {
	// allocate a new uniform buffer for the scene data
	// This should be deleted each frame.
	AllocatedBuffer gpuSceneDataBuffer = createBuffer("gpuSceneDataBuffer", sizeof(GPUSceneData), context->vmaAllocator, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// write the buffer
	memcpy(gpuSceneDataBuffer.info.pMappedData, &context->sceneData, sizeof(GPUSceneData));

	// Create global sceneData descriptor
	VkDescriptorSet globalDescriptor = getCurrentFrame(context).frameDescriptor.allocate(context->device, context->gpuSceneDataDescriptorLayout);

	getCurrentFrame(context).deletionQueue.push([gpuSceneDataBuffer, context]() {
		destroyBuffer(context->vmaAllocator, gpuSceneDataBuffer);
	});

	{
		DescriptorWriter writer;
		writer.writeBuffer(0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		writer.updateSet(context->device, globalDescriptor);
	}

	ComputePushConstants data = {
		.viewPosition = glm::vec4{ context->rendererState->mainCamera->position, 1.0f },
		.data2 = glm::vec4(context->rendererState->rendererStats.frameCount, 0.0f, 0.0f, 0.0f)
	};

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->gbufferPipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->gbufferPipelineLayout, 0, 1, &globalDescriptor, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->gbufferPipelineLayout, 1, 1, &context->gbufferDescriptorSet, 0, nullptr);

	vkCmdPushConstants(commandBuffer, context->gbufferPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &data);

	auto width = context->sceneDrawImage.imageExtent.width;
	auto height = context->sceneDrawImage.imageExtent.height;
	vkCmdDispatch(commandBuffer, std::ceil(width / 16.0), std::ceil(height / 16.0), 1);
}

void blitGBuffer(VulkanRendererContext* context, VkCommandBuffer commandBuffer) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->resolvePipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, context->resolvePipelineLayout, 0, 1, &context->resolveDescriptorSet, 0, nullptr);

	vkCmdPushConstants(commandBuffer, context->resolvePipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(uint32_t), &context->gbufferDebugChannel);

	auto width = context->sceneDrawImage.imageExtent.width;
	auto height = context->sceneDrawImage.imageExtent.height;
	vkCmdDispatch(commandBuffer, std::ceil(width / 16.0), std::ceil(height / 16.0), 1);
}

void drawUI(VulkanRendererContext* context, VkCommandBuffer commandBuffer) {
	auto uiStart = std::chrono::system_clock::now();

	VkClearValue clearColor{
		.color = { 0.0, 0.0, 0.0, 1.0 }
	};

	for (auto& windowBatch : getCurrentFrame(context).uiWindowBatches) {
		auto window = windowBatch.window;
		VkRenderingAttachmentInfo colorAttachment = attachmentInfo(window->renderTarget.imageView, &clearColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		VkRenderingAttachmentInfo depthAttachment = depthAttachmentInfo(window->depthTarget.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

		VkExtent2D extent{
			.width = (uint32_t)window->swapchain.extent.width,
			.height = (uint32_t)window->swapchain.extent.height,
		};
		VkRenderingInfo renderInfo = renderingInfo({ .extent = extent }, &colorAttachment, &depthAttachment);
		vkCmdBeginRendering(commandBuffer, &renderInfo);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		scissor.extent.width = extent.width;
		scissor.extent.height = extent.height;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		for (auto& drawBatch : windowBatch.drawBatches) {
			auto materialInstance = drawBatch.material;
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materialInstance.material->pipeline);

			for (uint32_t i = 0; i < materialInstance.material->layouts.size(); ++i) {
				auto layout = materialInstance.material->layouts.at(i);
				auto instanceDescriptorSet = materialInstance.descriptorSets.at(i);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materialInstance.material->pipelineLayout, layout.set, 1, &instanceDescriptorSet, 0, nullptr);
			}

			// TODO: Should be included as part of a Batch
			UIPushConstants uiPushConstants{};
			uiPushConstants.vertexBuffer = drawBatch.meshBuffers.vertexBufferAddress;
			uiPushConstants.screenWidth = static_cast<float>(extent.width);
			uiPushConstants.screenHeight = static_cast<float>(extent.height);

			vkCmdBindIndexBuffer(commandBuffer, drawBatch.meshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdPushConstants(commandBuffer, materialInstance.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UIPushConstants), &uiPushConstants);
			vkCmdDrawIndexedIndirect(commandBuffer, drawBatch.commands.buffer.buffer, drawBatch.commands.offset, drawBatch.commands.size, drawBatch.commands.stride);
		}

		vkCmdEndRendering(commandBuffer);
	}

	auto uiEnd = std::chrono::system_clock::now();
	auto uiElapsed = std::chrono::duration_cast<std::chrono::microseconds>(uiEnd - uiStart);

	context->rendererState->rendererStats.uiFrametimeAvg = context->rendererState->rendererStats.uiFrametimeAvg * 0.95 + (static_cast<float>(uiElapsed.count()) / 1000.0f) * 0.05;
}

void drawTerrain(VulkanRendererContext* context, VkCommandBuffer commandBuffer) {
	context->rendererState->rendererStats.drawCallCount = 0;

	auto start = std::chrono::system_clock::now();

	VkClearValue clearColor{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderingAttachmentInfo colorAttachment = attachmentInfo(context->sceneDrawImage.imageView, &clearColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depthAttachment = depthAttachmentInfo(context->sceneDepthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = renderingInfo({ .extent = { .width = context->sceneDrawImage.imageExtent.width, .height = context->sceneDrawImage.imageExtent.height } }, &colorAttachment, &depthAttachment);
	vkCmdBeginRendering(commandBuffer, &renderInfo);

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(context->sceneDrawImage.imageExtent.width);
	viewport.height = static_cast<float>(context->sceneDrawImage.imageExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = context->sceneDrawImage.imageExtent.width;
	scissor.extent.height = context->sceneDrawImage.imageExtent.height;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Setup global scene data descriptor
	AllocatedBuffer gpuSceneDataBuffer = createBuffer("gpuSceneDataBuffer", sizeof(GPUSceneData), context->vmaAllocator, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// write the buffer
	void* sceneUniformData{};
	memcpy(gpuSceneDataBuffer.info.pMappedData, &context->sceneData, sizeof(GPUSceneData));

	// Create global sceneData descriptor
	VkDescriptorSet globalDescriptor = getCurrentFrame(context).frameDescriptor.allocate(context->device, context->gpuSceneDataDescriptorLayout);

	getCurrentFrame(context).deletionQueue.push([gpuSceneDataBuffer, context]() {
		destroyBuffer(context->vmaAllocator, gpuSceneDataBuffer);
	});

	{
		DescriptorWriter writer;
		writer.writeBuffer(0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		writer.updateSet(context->device, globalDescriptor);
	}

	// create a draw command for each chunk
	std::vector<MeshIndirectCommand> drawCommands;
	std::vector<MeshDraw> drawData{};
	uint32_t drawId{ 0 };
	for (auto& chunk : context->voxelTerrain.chunks) {
		MeshIndirectCommand c{
			.drawId = drawId,
			.command = {
				.indexCount = chunk.indexCount,
				.instanceCount = 1,
				.firstIndex = chunk.firstIndex,
				.vertexOffset = chunk.vertexOffset,
				.firstInstance = drawId }
		};
		drawCommands.push_back(c);
		drawData.push_back({ .transform = chunk.transform, .materialIndex = 0 });
		drawId++;
	}

	VkDescriptorSet terrainBatchDescriptor = getCurrentFrame(context).frameDescriptor.allocate(context->device, context->voxelDescriptorLayout);

	auto commandsBuffer = createBuffer("voxelCommandsBuffer", sizeof(MeshIndirectCommand) * drawCommands.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(commandsBuffer.info.pMappedData, drawCommands.data(), sizeof(MeshIndirectCommand) * drawCommands.size());

	auto drawsBuffer = createBuffer("voxelTransformBuffer", sizeof(MeshDraw) * drawData.size(), context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	memcpy(drawsBuffer.info.pMappedData, drawData.data(), sizeof(MeshDraw) * drawData.size());

	getCurrentFrame(context).deletionQueue.push([commandsBuffer, drawsBuffer, context]() {
		destroyBuffer(context->vmaAllocator, commandsBuffer);
		destroyBuffer(context->vmaAllocator, drawsBuffer);
	});

	{
		DescriptorWriter writer;
		writer.writeBuffer(0, commandsBuffer.buffer, drawCommands.size() * sizeof(MeshIndirectCommand), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writer.writeBuffer(1, drawsBuffer.buffer, drawData.size() * sizeof(MeshDraw), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		writer.updateSet(context->device, terrainBatchDescriptor);
	}

	auto voxelPipeline = context->voxelWireframeActive ? context->voxelWireframePipeline : context->voxelPipeline;

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, voxelPipeline.pipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, voxelPipeline.layout, 0, 1, &globalDescriptor, 0, nullptr);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, voxelPipeline.layout, 1, 1, &terrainBatchDescriptor, 0, nullptr);

	GPUDrawPushConstants pushConstants{};
	pushConstants.vertexBuffer = context->voxelMeshBuffers.vertexBufferAddress;
	pushConstants.viewPosition = glm::vec4(context->rendererState->mainCamera->position, 1.0f);

	vkCmdBindIndexBuffer(commandBuffer, context->voxelMeshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdPushConstants(commandBuffer, voxelPipeline.layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
	vkCmdDrawIndexedIndirect(commandBuffer, commandsBuffer.buffer, offsetof(MeshIndirectCommand, command), drawCommands.size(), sizeof(MeshIndirectCommand));

	vkCmdEndRendering(commandBuffer);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	context->rendererState->rendererStats.drawCallCount = static_cast<int32_t>(drawCommands.size());

	context->rendererState->rendererStats.meshDrawTimeAvg = context->rendererState->rendererStats.meshDrawTimeAvg * 0.95 + (static_cast<float>(elapsed.count()) / 1000.0f) * 0.05;
}

void drawGeometry(VulkanRendererContext* context, VkCommandBuffer commandBuffer) {
	context->rendererState->rendererStats.drawCallCount = 0;

	auto start = std::chrono::system_clock::now();

	VkClearValue clearColor{ .color = { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkRenderingAttachmentInfo colorAttachment = attachmentInfo(context->sceneDrawImage.imageView, &clearColor, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingAttachmentInfo depthAttachment = depthAttachmentInfo(context->sceneDepthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = renderingInfo({ .extent = { .width = context->sceneDrawImage.imageExtent.width, .height = context->sceneDrawImage.imageExtent.height } }, &colorAttachment, &depthAttachment);
	vkCmdBeginRendering(commandBuffer, &renderInfo);

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(context->sceneDrawImage.imageExtent.width);
	viewport.height = static_cast<float>(context->sceneDrawImage.imageExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = context->sceneDrawImage.imageExtent.width;
	scissor.extent.height = context->sceneDrawImage.imageExtent.height;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	uint32_t drawCallCount{};

	// Process this frames draw batches
	for (auto& drawBatch : getCurrentFrame(context).drawBatches) {
		drawCallCount += drawBatch.commands.size;

		auto materialInstance = drawBatch.material;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materialInstance.material->pipeline);

		for (uint32_t i = 0; i < materialInstance.material->layouts.size(); ++i) {
			auto layout = materialInstance.material->layouts.at(i);
			auto instanceDescriptorSet = materialInstance.descriptorSets.at(i);

			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, materialInstance.material->pipelineLayout, layout.set, 1, &instanceDescriptorSet, 0, nullptr);
		}

		// TODO: Should be included as part of a Batch
		GPUDrawPushConstants pushConstants{};
		pushConstants.vertexBuffer = drawBatch.meshBuffers.vertexBufferAddress;
		pushConstants.viewPosition = glm::vec4(context->rendererState->mainCamera->position, 1.0f);

		vkCmdBindIndexBuffer(commandBuffer, drawBatch.meshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(commandBuffer, materialInstance.material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
		vkCmdDrawIndexedIndirect(commandBuffer, drawBatch.commands.buffer.buffer, drawBatch.commands.offset, drawBatch.commands.size, drawBatch.commands.stride);
	}

	context->rendererState->rendererStats.drawCallCount = static_cast<int32_t>(drawCallCount);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	vkCmdEndRendering(commandBuffer);

	context->rendererState->rendererStats.meshDrawTimeAvg = context->rendererState->rendererStats.meshDrawTimeAvg * 0.95 + (static_cast<float>(elapsed.count()) / 1000.0f) * 0.05;
}

void initDescriptors(VulkanRendererContext* context) {
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 0.7 },
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .ratio = 0.3 },
	};

	context->globalDescriptorAllocator.init(context->device, 1000, sizes);
	context->mainDeletionQueue.push([context]() {
		context->globalDescriptorAllocator.destroyPools(context->device);
	});

	// Compute stage image descriptor
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		context->drawImageDescriptorLayout = builder.build(context->device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	context->drawImageDescriptors = context->globalDescriptorAllocator.allocate(context->device, context->drawImageDescriptorLayout);

	{
		DescriptorWriter writer;
		writer.writeImage(0, context->sceneDrawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writer.updateSet(context->device, context->drawImageDescriptors);
	}

	// Global UBO
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		context->gpuSceneDataDescriptorLayout = builder.build(context->device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT);
	}

	// Terrain
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		context->voxelDescriptorLayout = builder.build(context->device);
	}

	context->mainDeletionQueue.push([context]() {
		vkDestroyDescriptorSetLayout(context->device, context->drawImageDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(context->device, context->gpuSceneDataDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(context->device, context->voxelDescriptorLayout, nullptr);
	});

	for (auto& frame : context->frames) {
		// create a descriptor pool
		std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes = {
			{ .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 3 },
			{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .ratio = 3 },
			{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 3 },
			{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .ratio = 4 },
		};

		frame.frameDescriptor = {};
		frame.frameDescriptor.init(context->device, 10000, frame_sizes);

		context->mainDeletionQueue.push([context, &frame]() {
			frame.frameDescriptor.destroyPools(context->device);
		});
	}
}

void initPipelines(VulkanRendererContext* context) {
	VkPipelineCacheCreateInfo cacheCreateInfo{ .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
	vkCreatePipelineCache(context->device, &cacheCreateInfo, nullptr, &context->pipelineCache);

	initMeshPipelines(context);
	initUIPipeline(context);
	initFontPipeline(context);
	initViewportPipeline(context);
	initVoxelPipeline(context);
}

void initUIPipeline(VulkanRendererContext* context) {
	context->uiMaterial = createMaterial(context, "res/materials/UI.Default.json");

	context->mainDeletionQueue.push([context]() {
		destroyMaterial(context, context->uiMaterial);
	});
}

void initViewportPipeline(VulkanRendererContext* context) {
	context->uiViewportMaterial = createMaterial(context, "res/materials/UIViewport.Default.json");

	// TODO(piero): This material has a bindless texture buffer. Because of the way the system works now, we need a material instance to be able to write to a descriptor set,
	//              even though the material already holds a descriptor set (since `createMaterial` initializes any "global" descriptor sets needed when creating the material)
	//              Should we allow writing to base materials? This would work because when creating an instance, we copy over any existing descriptor sets, so all instances of the
	//              material would just share the same set (and the layout of course). Which seems to be exactly what we want since its a "global" texture array for this material.
	context->uiViewportMaterialInstance = createMaterialInstance(&context->uiViewportMaterial);

	context->mainDeletionQueue.push([context]() {
		destroyMaterial(context, context->uiViewportMaterial);
	});
}

void initFontPipeline(VulkanRendererContext* context) {
	context->uiTextMaterial = createMaterial(context, "res/materials/UIText.Default.json");

	context->mainDeletionQueue.push([context]() {
		destroyMaterial(context, context->uiTextMaterial);
	});
}

void initVoxelPipeline(VulkanRendererContext* context) {
	VkShaderModule voxelFragShader{};
	if (!loadShaderModule("res/shaders/voxels.frag.spv", context->device, &voxelFragShader)) {
		std::cout << std::format("Error when building the voxel fragment shader module") << '\n';
	}

	VkShaderModule voxelVertexShader{};
	if (!loadShaderModule("res/shaders/voxels.vert.spv", context->device, &voxelVertexShader)) {
		std::cout << std::format("Error when building the voxel vertex shader module") << '\n';
	}

	VkPushConstantRange voxelPushConstants{};
	voxelPushConstants.offset = 0;
	voxelPushConstants.size = sizeof(GPUDrawPushConstants);
	voxelPushConstants.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayout, 2> layouts = {
		context->gpuSceneDataDescriptorLayout,
		context->voxelDescriptorLayout
	};

	VkPipelineLayoutCreateInfo voxelLayoutInfo = pipelineLayoutCreateInfo();
	voxelLayoutInfo.pPushConstantRanges = &voxelPushConstants;
	voxelLayoutInfo.pushConstantRangeCount = 1;
	voxelLayoutInfo.setLayoutCount = layouts.size();
	voxelLayoutInfo.pSetLayouts = layouts.data();

	VK_CHECK(vkCreatePipelineLayout(context->device, &voxelLayoutInfo, nullptr, &context->voxelPipeline.layout));
	VK_CHECK(vkCreatePipelineLayout(context->device, &voxelLayoutInfo, nullptr, &context->voxelWireframePipeline.layout));

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.setPipelineLayout(context->voxelPipeline.layout);
	pipelineBuilder.setShaders(voxelVertexShader, voxelFragShader);
	pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	pipelineBuilder.setMultisamplingNone();
	pipelineBuilder.disableBlending();
	pipelineBuilder.enableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

	// render format
	pipelineBuilder.setColorAttachmentFormat(context->sceneDrawImage.imageFormat);
	pipelineBuilder.setDepthFormat(context->sceneDepthImage.imageFormat);

	context->voxelPipeline.pipeline = pipelineBuilder.buildPipeline(context->device, context->pipelineCache);

	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_LINE);
	context->voxelWireframePipeline.pipeline = pipelineBuilder.buildPipeline(context->device, context->pipelineCache);

	context->mainDeletionQueue.push([context] {
		vkDestroyPipelineLayout(context->device, context->voxelPipeline.layout, nullptr);
		vkDestroyPipelineLayout(context->device, context->voxelWireframePipeline.layout, nullptr);

		vkDestroyPipeline(context->device, context->voxelPipeline.pipeline, nullptr);
		vkDestroyPipeline(context->device, context->voxelWireframePipeline.pipeline, nullptr);
	});

	vkDestroyShaderModule(context->device, voxelFragShader, nullptr);
	vkDestroyShaderModule(context->device, voxelVertexShader, nullptr);
}

void immediateSubmit(VulkanRendererContext* context, std::function<void(VkCommandBuffer cmd)>&& function) {
	VK_CHECK(vkResetFences(context->device, 1, &context->immFence));
	VK_CHECK(vkResetCommandBuffer(context->immCommandBuffer, 0));

	VkCommandBuffer cmd = context->immCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = commandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = submitInfo(&cmdinfo, nullptr, nullptr);

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(context->graphicsQueue, 1, &submit, context->immFence));

	VK_CHECK(vkWaitForFences(context->device, 1, &context->immFence, true, 9999999999));
}

void initBindlessTextureDescriptor(VulkanRendererContext* context, VkDescriptorSetLayout& descriptorSetLayout, VkDescriptorSet& descriptorSet) {
	constexpr uint32_t maxBindlessTextureResources = 1000;

	VkDescriptorPool pool{};

	VkDescriptorPoolSize poolSizesBindless[] = {
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = maxBindlessTextureResources }
	};

	VkDescriptorPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = poolSizesBindless;

	VK_CHECK(vkCreateDescriptorPool(context->device, &poolInfo, nullptr, &pool));

	VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

	VkDescriptorSetLayoutBinding layoutBinding;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBinding.descriptorCount = maxBindlessTextureResources;
	layoutBinding.binding = 0;
	layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBindingFlagsCreateInfo extendedInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO, nullptr };
	extendedInfo.bindingCount = 1;
	extendedInfo.pBindingFlags = &bindlessFlags;

	VkDescriptorSetLayoutCreateInfo bindlessTextureLayoutInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	bindlessTextureLayoutInfo.bindingCount = 1;
	bindlessTextureLayoutInfo.pBindings = &layoutBinding;
	bindlessTextureLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	bindlessTextureLayoutInfo.pNext = &extendedInfo;

	vkCreateDescriptorSetLayout(context->device, &bindlessTextureLayoutInfo, nullptr, &descriptorSetLayout);

	VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
	countInfo.descriptorSetCount = 1;
	countInfo.pDescriptorCounts = &maxBindlessTextureResources;

	VkDescriptorSetAllocateInfo setAllocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	setAllocateInfo.descriptorPool = pool;
	setAllocateInfo.descriptorSetCount = 1;
	setAllocateInfo.pSetLayouts = &descriptorSetLayout;

	setAllocateInfo.pNext = &countInfo;

	VK_CHECK(vkAllocateDescriptorSets(context->device, &setAllocateInfo, &descriptorSet));

	context->mainDeletionQueue.push([context, pool]() {
		vkDestroyDescriptorPool(context->device, pool, nullptr);
	});
}

void initMeshPipelines(VulkanRendererContext* context) {
	context->opaqueMaterial = createMaterial(context, "res/materials/Mesh_Opaque.Default.json", true);
	context->doubleSidedMaterial = createMaterial(context, "res/materials/Mesh_DoubleSided.Default.json", true);
	context->transparentMaterial = createMaterial(context, "res/materials/Mesh_Transparent.Default.json", true);

	context->mainDeletionQueue.push([context]() {
		destroyMaterial(context, context->opaqueMaterial);
		destroyMaterial(context, context->doubleSidedMaterial);
		destroyMaterial(context, context->transparentMaterial);
	});
}

void updateScene(VulkanRendererContext* context, float deltaTime) {
	auto start = std::chrono::system_clock::now();

	context->rendererState->mainCamera->update(deltaTime);

	context->sceneData.view = context->rendererState->mainCamera->getViewMatrix();
	context->sceneData.proj = context->rendererState->mainCamera->getPerspectiveProjection();

	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	context->sceneData.proj[1][1] *= -1;
	context->sceneData.viewproj = context->sceneData.proj * context->sceneData.view;

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	context->rendererState->rendererStats.sceneUpdateTimeAvg = context->rendererState->rendererStats.sceneUpdateTimeAvg * 0.95 + (static_cast<float>(elapsed.count()) / 1000.0f) * 0.05;
}

void updateFontData(VulkanRendererContext* context) {
	// Update uniform for each window
	for (auto& window : PrimalEngine::get().windows) {
		FontUniformData fontUniformData{};
		fontUniformData.outline = 0.0f;
		fontUniformData.view = glm::mat4(1.0f);
		auto w = static_cast<float>(window.width);
		auto h = static_cast<float>(window.height);
		fontUniformData.projection = glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);

		memcpy(window.fontData.info.pMappedData, &fontUniformData, sizeof(FontUniformData));
	}
}

void initFontData(VulkanRendererContext* context) {
	context->sourceCodeFont = loadFontSDF("SauceCodePro-Light", "res/fonts/SauceCodePro-Light.png", "res/fonts/SauceCodePro-Light.json");
	// arialFont = loadFontSDF("Arial", "res/fonts/arial.png", "res/fonts/arial.json");

	for (auto& window : PrimalEngine::get().windows) {
		window.fontData = createBuffer(std::format("fontUniformBuffer-{}", window.id), sizeof(FontUniformData), context->vmaAllocator, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	}

	auto extents = VkExtent3D{
		context->sourceCodeFont.image.width,
		context->sourceCodeFont.image.height,
		1
	};
	context->sourceCodeFontTexture = createImage("SourceCodeFont-Image", context->sourceCodeFont.image.data, extents, context, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);
	context->sourceCodeFontTexture.sampler = context->defaultSamplerLinear;

	updateFontData(context);
}

void updateUIData(VulkanRendererContext* context) {
	// Update uniform for each window
	for (auto& window : PrimalEngine::get().windows) {
		UIUniformData uiUniformData{};
		uiUniformData.view = glm::mat4(1.0f);
		auto w = static_cast<float>(window.width);
		auto h = static_cast<float>(window.height);
		uiUniformData.projection = glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);

		memcpy(window.uiData.info.pMappedData, &uiUniformData, sizeof(UIUniformData));
	}

	auto& rendererState = context->rendererState;
	auto& sceneData = context->sceneData;

	auto stats = std::format("Frametime: {:.2f}ms | GPU: {:.2f}ms | UI: {:.4f}ms | Triangles: {:.2f}M | DrawCall: {}",
		rendererState->rendererStats.frametime,
		rendererState->rendererStats.frameGpuTimeAvg,
		rendererState->rendererStats.uiFrametimeAvg,
		rendererState->rendererStats.triangleCount * 1e-6,
		rendererState->rendererStats.drawCallCount);

	auto otherStats = std::format("DrawBatchGen: {:.4f}us | UIDrawBatchGen: {:.4f}us | EntityFlatten: {:.4f}us | UILayout: {:.4f}us | SceneUpdate: {:.4f}us | MeshDraw: {:.4f}us",
		rendererState->rendererStats.drawBatchGenerationTimeAvg,
		rendererState->rendererStats.uiDrawBatchGenerationTimeAvg,
		rendererState->rendererStats.entityFlattenTimeAvg,
		rendererState->rendererStats.uiLayoutTimeAvg,
		rendererState->rendererStats.sceneUpdateTimeAvg,
		rendererState->rendererStats.meshDrawTimeAvg);

	auto cameraPosition = std::format("Camera Pos: {:.2f} {:.2f} {:.2f}",
		rendererState->mainCamera->position.x,
		rendererState->mainCamera->position.y,
		rendererState->mainCamera->position.z);

	auto sunDirection = std::format("Sun Direction: {:.2f} {:.2f} {:.2f} {:.2f}",
		sceneData.sunlightDirection.x,
		sceneData.sunlightDirection.y,
		sceneData.sunlightDirection.z,
		sceneData.sunlightDirection.w);

	auto sunColor = std::format("Sun Color: {:.2f} {:.2f} {:.2f} {:.2f}",
		sceneData.sunlightColor.x,
		sceneData.sunlightColor.y,
		sceneData.sunlightColor.z,
		sceneData.sunlightColor.w);

	UI::setFont(&context->sourceCodeFont);

	auto start = std::chrono::high_resolution_clock::now();

	UI::beginFrame();

	if (context->fullScreen) {
		UI::beginWindow(rendererState->window);

		// Title bar
		UI::openElement();
		UI::pushBox({ .width = { .size = static_cast<float>(rendererState->window->width), .sizingMode = UI::UISizingMode::STATIC },
			.height = { .size = 80.0f, .sizingMode = UI::UISizingMode::STATIC },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.678f, 0.678f, 0.678f, 1.0f } });

		// Show renderer stats
		UI::openElement();
		UI::pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .size = 80.0f, .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::VERTICAL,
			.backgroundColor = { 0.0f, 0.0f, 0.0f, 0.0f },
			.padding = 10.0f,
			.childGap = 10.0f });
		UI::openTextElement();
		UI::pushText({ .text = stats });
		UI::closeTextElement();

		UI::openTextElement();
		UI::pushText({ .text = otherStats });
		UI::closeTextElement();
		UI::closeElement();

		UI::closeElement();

		// TODO(piero): Refactor this when implementing GROW sizing for UI
		UI::viewport({ .id = 1000,
			.width = static_cast<float>(context->rendererState->window->width),
			.height = static_cast<float>(context->rendererState->window->height - 80),
			.textureId = context->sceneTextureId });

		UI::endWindow();
	} else {
		// TODO(piero): Refactor this
		auto testWindow = &PrimalEngine::get().windows[1];

		UI::beginWindow(rendererState->window);

		// Title bar
		UI::openElement();
		UI::pushBox({ .width = { .size = 2048.0f, .sizingMode = UI::UISizingMode::STATIC },
			.height = { .size = 80.0f, .sizingMode = UI::UISizingMode::STATIC },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.678f, 0.678f, 0.678f, 1.0f } });

		// Show renderer stats
		UI::openElement();
		UI::pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .size = 80.0f, .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::VERTICAL,
			.backgroundColor = { 0.0f, 0.0f, 0.0f, 0.0f },
			.padding = 10.0f,
			.childGap = 10.0f });

		UI::openTextElement();
		UI::pushText({ .text = stats });
		UI::closeTextElement();

		UI::openTextElement();
		UI::pushText({ .text = otherStats });
		UI::closeTextElement();
		UI::closeElement();

		UI::closeElement();

		UI::openElement();
		UI::pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.0f, 1.0f, 0.0f, 0.0f } });

		// Viewport + Asset browser
		UI::openElement();
		UI::pushPanel({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::VERTICAL,
			.backgroundColor = { 0.0f, 1.0f, 0.0f, 0.0f } });

		// Viewport
		UI::pushDockSpace({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.3294f, 0.3294f, 0.3294f, 1.0f } });

		UI::viewport({ .id = 1000,
			.width = 1448.0f,
			.height = 700.0f,
			.textureId = context->sceneTextureId });

		UI::closeDockSpaceElement();

		// Asset browser
		UI::pushDockSpace({ .width = { .size = 1448.0f, .sizingMode = UI::UISizingMode::STATIC },
			.height = { .size = 300.0f, .sizingMode = UI::UISizingMode::STATIC },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.3294f, 0.3294f, 0.3294f, 1.0f } });

		UI::closeDockSpaceElement();
		UI::closeElement();

		// Info Panel
		UI::pushDockSpace({ .width = { .size = 600.0f, .sizingMode = UI::UISizingMode::STATIC },
			.height = { .size = 980.0f, .sizingMode = UI::UISizingMode::STATIC },
			.layoutDirection = UI::UILayoutDirection::VERTICAL,
			.backgroundColor = { 0.41960784313f, 0.41960784313f, 0.41960784313f, 1.0f },
			.padding = 10.0f,
			.childGap = 10.0f });

		UI::sliderFloat3(&rendererState->mainCamera->position);
		UI::sliderFloat4(&sceneData.sunlightDirection, 0.0f, 1.0f);
		UI::checkbox(&context->testBool);

		UI::openElement();
		UI::pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f },
			.padding = 10.0f,
			.childGap = 20.0f });

		UI::pushCircleFilled(80.0f, 32, { 1.0f, 0.0f, 0.0f, 1.0f });
		UI::pushCircle(80.0f, 32, 10.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
		UI::closeElement();

		UI::sliderFloat4(&sceneData.sunlightColor, 0.0f, 1.0f);

		UI::openTextElement();
		UI::pushText({ .padding = UI::UIPadding{ 0.0f, 20.0f, 0.0f, 0.0f }, .text = "Noise Params" });
		UI::closeTextElement();

		UI::openTextElement();
		UI::pushText({ .text = "Height Scale" });
		UI::closeTextElement();
		UI::sliderFloat(&context->terrainParams.heightScale, 0.1f, 100.0f);

		UI::openTextElement();
		UI::pushText({ .text = "Height Multiplier" });
		UI::closeTextElement();
		UI::sliderFloat(&context->terrainParams.heightMultiplier, 0.1f, 100.0f);

		UI::openTextElement();
		UI::pushText({ .text = "Noise Scale" });
		UI::closeTextElement();
		UI::sliderFloat(&context->terrainParams.noiseScale, 0.1f, 100.0f);

		UI::openTextElement();
		UI::pushText({ .text = "Wireframe Mode" });
		UI::closeTextElement();
		UI::checkbox(&context->voxelWireframeActive);

		UI::closeDockSpaceElement();
		UI::closeElement();

		UI::endWindow();

		UI::beginWindow(testWindow);

		UI::openElement();
		UI::pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.0f, 1.0f, 0.0f, 0.0f } });

		UI::pushDockSpace({ .width = { .size = 400.0f, .sizingMode = UI::UISizingMode::STATIC },
			.height = { .size = 800.0f, .sizingMode = UI::UISizingMode::STATIC },
			.layoutDirection = UI::UILayoutDirection::VERTICAL,
			.backgroundColor = { 0.41960784313f, 0.41960784313f, 0.41960784313f, 1.0f },
			.padding = 10.0f,
			.childGap = 10.0f });

		UI::sliderFloat3(&rendererState->mainCamera->position);
		UI::sliderFloat4(&sceneData.sunlightDirection, 0.0f, 1.0f);

		UI::openElement();
		UI::pushBox({ .width = { .sizingMode = UI::UISizingMode::FIT },
			.height = { .sizingMode = UI::UISizingMode::FIT },
			.layoutDirection = UI::UILayoutDirection::HORIZONTAL,
			.backgroundColor = { 0.0f, 0.0f, 0.0f, 1.0f },
			.padding = 10.0f,
			.childGap = 20.0f });

		UI::pushCircleFilled(50.0f, 32, { 1.0f, 0.0f, 0.0f, 1.0f });
		UI::pushCircle(50.0f, 32, 10.0f, { 0.0f, 1.0f, 0.0f, 1.0f });
		UI::closeElement();

		UI::sliderFloat4(&sceneData.sunlightColor, 0.0f, 1.0f);
		UI::closeDockSpaceElement();
		UI::closeElement();


		UI::endWindow();
	}

	// NOTE(piero): We are not making a deep copy of this data. We are still referencing the arena memory.
	getCurrentFrame(context).uiWindowBatchCommands = UI::endFrame();

	auto uiLayoutTime = std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - start).count();
	context->rendererState->rendererStats.uiLayoutTimeAvg = context->rendererState->rendererStats.uiLayoutTimeAvg * 0.95 + uiLayoutTime * 0.05;
}

void initUI(VulkanRendererContext* context) {
	context->uiMemoryArena = MemoryArena_create(MEGABYTE(20));
	UI::initRenderContext(&context->uiMemoryArena);

	// Register image to UI system
	context->sceneTextureId = registerImage(context, &context->sceneDrawImage);

	writeUniform(context, &context->uiViewportMaterialInstance, 1, 0, context->sceneDrawImage.imageView, context->defaultSamplerLinear, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, context->sceneTextureId);

	for (auto& window : PrimalEngine::get().windows) {
		window.uiData = createBuffer(std::format("uiUniformBuffer-{}", window.id), sizeof(UIUniformData), context->vmaAllocator, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
	}

	updateUIData(context);
}

void setPointerState(uint32_t windowId, float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown) {
	UI::setPointerState(windowId, mouseX, mouseY, relMouseX, relMouseY, isPointerDown);
}

// TODO(piero): Rework this. Right now we are just using this to keep track of the indices in which we upload to the global descriptor for textures.
uint32_t registerImage(VulkanRendererContext* context, AllocatedImage* image) {
	// Image are registered starting from 1. 0 is the fallback image.
	auto index = context->registeredImages.size() + 1;
	context->registeredImages.push_back(image);
	return index;
}

/*
 * Create a staging buffer in CPU memory to hold the vertex + index buffer data.
 * Copy it to a GPU buffer.
 */
GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<UI::UIVertex> vertices, std::string name) {
	const auto vertexBufferSize = vertices.size() * sizeof(UI::UIVertex);
	const auto indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface{};

	// create vertex buffer
	newSurface.vertexBuffer = createBuffer(name + " MeshVertexBuffer", vertexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	// find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer
	};
	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(context->device, &deviceAdressInfo);

	// create index buffer
	newSurface.indexBuffer = createBuffer(name + " MeshIndexBuffer", indexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	AllocatedBuffer staging = createBuffer(name + " Mesh staging", vertexBufferSize + indexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	// copy vertex buffer
	memcpy(staging.info.pMappedData, vertices.data(), vertexBufferSize);
	// copy index buffer
	memcpy(static_cast<char*>(staging.info.pMappedData) + vertexBufferSize, indices.data(), indexBufferSize);

	immediateSubmit(context, [&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertexBufferSize;
		indexCopy.size = indexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
	});

	destroyBuffer(context->vmaAllocator, staging);

	return newSurface;
}

GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<Vertex> vertices, std::string name) {
	const auto vertexBufferSize = vertices.size() * sizeof(Vertex);
	const auto indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface{};

	// create vertex buffer
	newSurface.vertexBuffer = createBuffer(name + " MeshVertexBuffer", vertexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	// find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer
	};
	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(context->device, &deviceAdressInfo);

	// create index buffer
	newSurface.indexBuffer = createBuffer(name + " MeshIndexBuffer", indexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	AllocatedBuffer staging = createBuffer(name + " Mesh staging", vertexBufferSize + indexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	// copy vertex buffer
	memcpy(staging.info.pMappedData, vertices.data(), vertexBufferSize);
	// copy index buffer
	memcpy(static_cast<char*>(staging.info.pMappedData) + vertexBufferSize, indices.data(), indexBufferSize);

	immediateSubmit(context, [&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertexBufferSize;
		indexCopy.size = indexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
	});

	destroyBuffer(context->vmaAllocator, staging);

	return newSurface;
}

GPUMeshBuffers uploadMesh(VulkanRendererContext* context, std::span<uint32_t> indices, std::span<VoxelVertex> vertices, std::string name) {
	const auto vertexBufferSize = vertices.size() * sizeof(VoxelVertex);
	const auto indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface{};

	// create vertex buffer
	newSurface.vertexBuffer = createBuffer(name + " MeshVertexBuffer", vertexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	// find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer
	};
	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(context->device, &deviceAdressInfo);

	// create index buffer
	newSurface.indexBuffer = createBuffer(name + " MeshIndexBuffer", indexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	AllocatedBuffer staging = createBuffer(name + " Mesh staging", vertexBufferSize + indexBufferSize, context->vmaAllocator, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	// copy vertex buffer
	memcpy(staging.info.pMappedData, vertices.data(), vertexBufferSize);
	// copy index buffer
	memcpy(static_cast<char*>(staging.info.pMappedData) + vertexBufferSize, indices.data(), indexBufferSize);

	immediateSubmit(context, [&](VkCommandBuffer cmd) {
		VkBufferCopy vertexCopy{ 0 };
		vertexCopy.dstOffset = 0;
		vertexCopy.srcOffset = 0;
		vertexCopy.size = vertexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.vertexBuffer.buffer, 1, &vertexCopy);

		VkBufferCopy indexCopy{ 0 };
		indexCopy.dstOffset = 0;
		indexCopy.srcOffset = vertexBufferSize;
		indexCopy.size = indexBufferSize;

		vkCmdCopyBuffer(cmd, staging.buffer, newSurface.indexBuffer.buffer, 1, &indexCopy);
	});

	destroyBuffer(context->vmaAllocator, staging);

	return newSurface;
}

}// namespace pm
