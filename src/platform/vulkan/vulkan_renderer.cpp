#include "config.h"
#include "ui/ui_types.h"
#include <algorithm>
#include <chrono>
#include <iterator>
#include <ratio>
#include <vulkan/vulkan_core.h>

#define VMA_LEAK_LOG_FORMAT(format, ...) \
	do {                                   \
		printf((format), __VA_ARGS__);       \
		printf("\n");                        \
	} while (false)

#define VMA_IMPLEMENTATION
#include "entity.h"
#include "platform/vulkan/vulkan_descriptor.h"
#include "platform/vulkan/vulkan_images.h"
#include "platform/vulkan/vulkan_loader.h"
#include "ui/primitives.h"
#include "vk_types.h"
#include "vulkan_pipeline.h"
#include "vulkan_renderer.h"
#include "vulkan_shader.h"
#include "vulkan_structures_helpers.h"
#include <SDL3/SDL_vulkan.h>
#include <array>
#include <cmath>
#include <glm/gtx/transform.hpp>
#include <vector>

namespace pm {

void VulkanRenderer::init(VulkanRendererConfig* state) {
	m_rendererState = state;
	initVulkan();
	initSwapchain();
	initCommands();
	initSyncStructures();
	initFontData();
	initUI();
	initDescriptors();
	initBindlessTextureDescriptor();
	initPipelines();
	initQueryPools();

	m_materialCache = MaterialCache_init();

	initDefaultData();

	const std::string modelPath = { "res/models/bistro/bistro_ktx2.glb" };

	auto start = std::chrono::system_clock::now();
	auto loadedGLTF = loadGLTF(this, modelPath);
	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << std::format("Loaded in {:.4f} seconds\n", static_cast<float>(elapsed.count()) / 1000.0f);

	assert(loadedGLTF.has_value());

	loadedModels["testModel"] = loadedGLTF.value();
}

void VulkanRenderer::initQueryPools() {
	timestampPool = createQueryPool(m_device, 128, VK_QUERY_TYPE_TIMESTAMP);
	assert(timestampPool);

	pipelineStatisticsPool = createQueryPool(m_device, 1, VK_QUERY_TYPE_PIPELINE_STATISTICS);
	assert(pipelineStatisticsPool);

	m_mainDeletionQueue.push([&]() {
		vkDestroyQueryPool(m_device, timestampPool, nullptr);
		vkDestroyQueryPool(m_device, pipelineStatisticsPool, nullptr);
	});
}

void VulkanRenderer::resizeSwapchain() {
	vkDeviceWaitIdle(m_device);

	destroySwapchain();

	int w{}, h{};
	SDL_GetWindowSize(m_rendererState->window, &w, &h);

	int displayWidth{}, displayHeight{};
	SDL_GetWindowSizeInPixels(m_rendererState->window, &displayWidth, &displayHeight);

	m_rendererState->windowExtent.width = w;
	m_rendererState->windowExtent.height = h;
	m_renderScale = static_cast<float>(displayWidth) / static_cast<float>(w);

	createSwapchain(m_rendererState->windowExtent.width, m_rendererState->windowExtent.height);

	UI::onResizeCallback(static_cast<float>(m_rendererState->windowExtent.width), static_cast<float>(m_rendererState->windowExtent.height));

	m_rendererState->resizeRequested = false;
}

void VulkanRenderer::initDefaultData() {
	// 3 default textures, white, grey, black. 1 pixel each
	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	whiteImage = createImage("whiteImage", (void*)&white, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	uint32_t grey = glm::packUnorm4x8(glm::vec4(0.66f, 0.66f, 0.66f, 1));
	greyImage = createImage("greyImage", (void*)&grey, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	blackImage = createImage("blackImage", (void*)&black, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	// checkerboard image
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	std::array<uint32_t, 16 * 16> pixels{};// for 16x16 checkerboard texture
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			pixels.at(y * 16 + x) = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}
	errorCheckerboardImage = createImage("errorCheckedboardImage", pixels.data(), VkExtent3D{ 16, 16, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	VkSamplerCreateInfo sampl = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

	sampl.magFilter = VK_FILTER_NEAREST;
	sampl.minFilter = VK_FILTER_NEAREST;

	vkCreateSampler(m_device, &sampl, nullptr, &defaultSamplerNearest);

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	vkCreateSampler(m_device, &sampl, nullptr, &defaultSamplerLinear);

	// Create global material data buffer
	// TODO: Using fixed size for now.
	globalMaterialDataBuffer = createBuffer("globalMaterialDataBuffer", sizeof(MaterialData) * 1001, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	auto sceneMaterialData = static_cast<MaterialData*>(globalMaterialDataBuffer.info.pMappedData);

	// Init default material
	auto defaultMaterial = Material_getDefaultMaterial();
	sceneMaterialData[0] = defaultMaterial.materialData;

	// Write default texture to descriptor set and set Material pass and pipeline
	writeBindlessTextureToGlobalDescriptor(bindlessTexturesDescriptorSet, errorCheckerboardImage, defaultSamplerLinear, 0);
	defaultMaterial.passType = MaterialPass::MainColor;
	defaultMaterial.pipeline = &opaquePipeline;

	// Write default material to cache
	MaterialCache_add(m_materialCache, 0, defaultMaterial);

	m_mainDeletionQueue.push([&]() {
		vkDestroySampler(m_device, defaultSamplerNearest, nullptr);
		vkDestroySampler(m_device, defaultSamplerLinear, nullptr);

		destroyImage(whiteImage);
		destroyImage(greyImage);
		destroyImage(blackImage);
		destroyImage(errorCheckerboardImage);
	});
}

void VulkanRenderer::initVulkan() {
	vkb::InstanceBuilder builder;

	// make the vulkan instance, with basic debug features
	auto inst = builder.set_app_name("Primal Engine")
								.request_validation_layers(m_rendererState->useValidationLayers)
								.use_default_debug_messenger()
								.require_api_version(1, 3, 0)
								.build();

	auto vkbInstance = inst.value();

	// grab the instance
	m_instance = vkbInstance.instance;
	m_debug_messenger = vkbInstance.debug_messenger;

	SDL_Vulkan_CreateSurface(m_rendererState->window, m_instance, nullptr, &m_surface);

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

	// Use VKBootstrap to select a gpu.
	// We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
	vkb::PhysicalDeviceSelector selector{ vkbInstance };
	vkb::PhysicalDevice physicalDevice = selector
																				 .set_minimum_version(1, 3)
																				 .set_required_features(features)
																				 .set_required_features_11(features11)
																				 .set_required_features_13(features13)
																				 .set_required_features_12(features12)
																				 .set_surface(m_surface)
																				 .add_required_extension("VK_EXT_scalar_block_layout")
																				 .select()
																				 .value();

	std::cout << std::format("GPU: {}\n", physicalDevice.name);

	// create the final vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a vulkan application
	m_device = vkbDevice.device;
	m_chosenGPU = physicalDevice.physical_device;

	// Make sure we can timestamp and get update period
	assert(physicalDevice.properties.limits.timestampComputeAndGraphics);
	physicalDeviceTimestampPeriod = physicalDevice.properties.limits.timestampPeriod;

	anisotropyEnabled = physicalDevice.features.samplerAnisotropy;
	maxSamplerAnisotropy = physicalDevice.properties.limits.maxSamplerAnisotropy;

	// Check supported native GPU formats for KTX2
	auto formatSupported = [&](VkFormat format) {
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice.physical_device, format, &formatProperties);
		return ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT) && (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT));
	};

	// Block compression
	if (physicalDevice.features.textureCompressionBC) {
		if (formatSupported(VK_FORMAT_BC7_SRGB_BLOCK)) {
			availableTargetFormats.emplace_back(KTX_TTF_BC7_RGBA);
			availableTargetFormatsNames.emplace_back("KTX_TTF_BC7_RGBA");
		}

		if (formatSupported(VK_FORMAT_BC3_SRGB_BLOCK)) {
			availableTargetFormats.emplace_back(KTX_TTF_BC3_RGBA);
			availableTargetFormatsNames.emplace_back("KTX_TTF_BC3_RGBA");
		}
	}

	// Adaptive scalable texture compression
	if (physicalDevice.features.textureCompressionASTC_LDR) {
		if (formatSupported(VK_FORMAT_ASTC_4x4_SRGB_BLOCK)) {
			availableTargetFormats.emplace_back(KTX_TTF_ASTC_4x4_RGBA);
			availableTargetFormatsNames.emplace_back("KTX_TTF_ASTC_4x4_RGBA");
		}
	}

	// Ericsson texture compression
	if (physicalDevice.features.textureCompressionETC2) {
		if (formatSupported(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK)) {
			availableTargetFormats.emplace_back(KTX_TTF_ETC2_RGBA);
			availableTargetFormatsNames.emplace_back("KTX_TTF_ETC2_RGBA");
		}
	}

	// Get graphics queue with VKBootstrap
	m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	// Create allocator using VMA
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_chosenGPU;
	allocatorInfo.device = m_device;
	allocatorInfo.instance = m_instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	vmaCreateAllocator(&allocatorInfo, &m_allocator);
}

void VulkanRenderer::initSwapchain() {
	createSwapchain(m_rendererState->windowExtent.width, m_rendererState->windowExtent.height);

	// For render image and depth iamge, we want to allocate them from gpu local memory
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// draw image size will match the window
	VkExtent3D drawImageExtent = {
		m_rendererState->windowExtent.width,
		m_rendererState->windowExtent.height,
		1
	};

	m_drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	m_drawImage.imageExtent = drawImageExtent;

	VkImageUsageFlags drawImageUsages{};
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = imageCreateInfo(m_drawImage.imageFormat, drawImageUsages, drawImageExtent);

	vmaCreateImage(m_allocator, &rimg_info, &rimg_allocinfo, &m_drawImage.image, &m_drawImage.allocation, nullptr);
	VkImageViewCreateInfo rview_info = imageViewCreateInfo(m_drawImage.imageFormat, m_drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(m_device, &rview_info, nullptr, &m_drawImage.imageView));

	// Depth image
	m_depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
	m_depthImage.imageExtent = drawImageExtent;
	VkImageUsageFlags depthImageUsages{};
	depthImageUsages |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

	VkImageCreateInfo dimg_info = imageCreateInfo(m_depthImage.imageFormat, depthImageUsages, drawImageExtent);

	vmaCreateImage(m_allocator, &dimg_info, &rimg_allocinfo, &m_depthImage.image, &m_depthImage.allocation, nullptr);
	VkImageViewCreateInfo dview_info = imageViewCreateInfo(m_depthImage.imageFormat, m_depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(m_device, &dview_info, nullptr, &m_depthImage.imageView));

	m_mainDeletionQueue.push([&]() {
		vkDestroyImageView(m_device, m_drawImage.imageView, nullptr);
		vmaDestroyImage(m_allocator, m_drawImage.image, m_drawImage.allocation);

		vkDestroyImageView(m_device, m_depthImage.imageView, nullptr);
		vmaDestroyImage(m_allocator, m_depthImage.image, m_depthImage.allocation);
	});
}

void VulkanRenderer::initCommands() {
	auto commandPoolInfo = commandPoolCreateInfo(m_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (auto& frame : m_frames) {
		VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &frame.m_commandPool));
		auto commandAllocateInfo = commandBufferAllocateInfo(frame.m_commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(m_device, &commandAllocateInfo, &frame.m_commandBuffer));
		m_mainDeletionQueue.push([frame, this]() { vkDestroyCommandPool(m_device, frame.m_commandPool, nullptr); });
	}

	// Create command buffer for immediate submits
	VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_immCommandPool));
	VkCommandBufferAllocateInfo cmdAllocInfo = commandBufferAllocateInfo(m_immCommandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_immCommandBuffer));
	m_mainDeletionQueue.push([this]() { vkDestroyCommandPool(m_device, m_immCommandPool, nullptr); });
}

void VulkanRenderer::initSyncStructures() {
	// create syncronization structures
	// one fence to control when the gpu has finished rendering the frame,
	// and 2 semaphores to syncronize rendering with swapchain
	// we want the fence to start signaled so we can wait on it on the first frame
	auto fenceCreate = fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	auto semaphoreCreate = semaphoreCreateInfo();


	VK_CHECK(vkCreateFence(m_device, &fenceCreate, nullptr, &m_immFence));
	m_mainDeletionQueue.push([this]() { vkDestroyFence(m_device, m_immFence, nullptr); });

	for (auto& frame : m_frames) {
		VK_CHECK(vkCreateFence(m_device, &fenceCreate, nullptr, &frame.m_renderFence));

		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreate, nullptr, &frame.m_swapchainSemaphore));
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreate, nullptr, &frame.m_renderSemaphore));
		m_mainDeletionQueue.push([frame, this] {
			vkDestroyFence(m_device, frame.m_renderFence, nullptr);
			vkDestroySemaphore(m_device, frame.m_swapchainSemaphore, nullptr);
			vkDestroySemaphore(m_device, frame.m_renderSemaphore, nullptr);
		});
	}
}

void VulkanRenderer::createSwapchain(uint32_t width, uint32_t height) {
	vkb::SwapchainBuilder swapchainBuilder{ m_chosenGPU, m_device, m_surface };

	m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
																	.set_desired_format(VkSurfaceFormatKHR{ .format = m_swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
#if VSYNC
																	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
#else
																	.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
#endif
																	.set_desired_extent(width, height)
																	.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
																	.build()
																	.value();

	m_swapchainExtent = vkbSwapchain.extent;
	m_swapchain = vkbSwapchain.swapchain;
	m_swapchainImages = vkbSwapchain.get_images().value();
	m_swapchainImageViews = vkbSwapchain.get_image_views().value();
}

void VulkanRenderer::destroySwapchain() {
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	// destroy swapchain resources
	for (auto& swapchainImageView : m_swapchainImageViews) {
		vkDestroyImageView(m_device, swapchainImageView, nullptr);
	}
}

void VulkanRenderer::cleanup() {
	vkDeviceWaitIdle(m_device);

	for (auto& [name, model] : loadedModels) {
		cleanupModel(this, model);
	}

	loadedModels.clear();

	for (auto& frame : m_frames) {
		frame.m_deletionQueue.flush();
	}

	m_mainDeletionQueue.flush();

	UI::cleanupRenderContext();

	destroySwapchain();
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vmaDestroyAllocator(m_allocator);
	vkDestroyDevice(m_device, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
	vkDestroyInstance(m_instance, nullptr);
}

std::vector<UIElement> VulkanRenderer::buildUIGeometry(std::vector<UI::UIRenderCommand>& renderCommands, std::vector<UIVertex>& vertices, std::vector<uint32_t>& indices) {
	std::vector<UIElement> elements;

	for (auto& renderCommand : renderCommands) {
		switch (renderCommand.commandType) {
		case pm::UI::UIRenderCommandType::RECTANGLE: {
			elements.push_back(UI::box(vertices, indices));
			break;
		}
		case pm::UI::UIRenderCommandType::TEXT: {
			elements.push_back(UI::text(renderCommand.text, static_cast<float>(fontSDF.width), fontChars, &vertices, &indices));
			break;
		}
		}
	}

	return elements;
}

void VulkanRenderer::buildUIDrawBatches(std::vector<UI::UIRenderCommand>& renderCommands) {
	auto start = std::chrono::system_clock::now();

	getCurrentFrame().uiDrawBatches.clear();

	std::vector<UIDrawData> uiDrawData;
	std::vector<glm::mat4> textTransformData;

	std::vector<UIIndirectCommand> uiDrawCommands;
	std::vector<UIIndirectCommand> textDrawCommands;

	DrawBatch uiDrawBatch{};
	DrawBatch textDrawBatch{};

	// TODO: generate vertex/index data
	std::vector<UIVertex> vertices;
	std::vector<uint32_t> indices;
	auto elements = buildUIGeometry(renderCommands, vertices, indices);

	// We use one Vertex/index buffer for all UI geometry
	auto uiGeometryBuffers = uploadMesh<UIVertex>(indices, vertices, "uiMeshBuffers");

	std::vector<UIMaterialData> uiMaterialData;

	uiDrawBatch.meshBuffers = uiGeometryBuffers;
	uiDrawBatch.pipeline = uiPipeline;
	uiDrawBatch.pipelineLayout = uiPipelineLayout;

	textDrawBatch.meshBuffers = uiGeometryBuffers;
	textDrawBatch.pipeline = fontPipeline;
	textDrawBatch.pipelineLayout = fontPipelineLayout;

	uint32_t uiDrawIdCount{ 0 }, textDrawIdCount{ 0 };

	for (uint32_t i = 0; i < elements.size(); ++i) {
		auto& renderCommand = renderCommands.at(i);
		auto& uiElement = elements.at(i);


		auto& bb = renderCommand.boundingBox;
		auto transform = glm::mat4{ 1.0f };

		switch (renderCommand.commandType) {
		case pm::UI::UIRenderCommandType::RECTANGLE: {
			transform = glm::translate(transform, glm::vec3(bb.x + bb.width / 2.0f, bb.y + bb.height / 2.0f, 0.0f));
			// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
			transform = glm::scale(transform, glm::vec3(bb.width / 2.0f, bb.height / 2.0f, 1.0f));
			// uiDrawCommands.push_back(getBoxDrawCommand(renderCommand));
			uiDrawCommands.push_back({ .drawId = uiDrawIdCount,
				.command = {
					.indexCount = uiElement.indexCount,
					.instanceCount = 1,
					.firstIndex = uiElement.firstIndex,
					.vertexOffset = uiElement.vertexOffset,
					.firstInstance = uiDrawIdCount } });
			uiDrawData.push_back({ .transform = transform, .materialIndex = uiDrawIdCount });
			uiMaterialData.push_back({ .backgroundColor = renderCommand.backgroundColor });
			uiDrawIdCount++;
			break;
		}
		case pm::UI::UIRenderCommandType::TEXT: {
			transform = glm::translate(transform, glm::vec3(bb.x, bb.y, 0.0f));
			// transform = glm::rotate(transform, glm::radians(uiElement.rotation), glm::vec3(0.0f, 0.0f, 1.0f));
			transform = glm::scale(transform, glm::vec3(1.0f, 1.0f, 1.0f));
			// textDrawCommands.push_back(getTextDrawCommand(renderCommand));
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

	auto uiDrawCommandsBuffer = createBuffer("uiIndirectCommandBuffer", sizeof(UIIndirectCommand) * uiDrawCommands.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	void* drawData = uiDrawCommandsBuffer.allocation->GetMappedData();
	memcpy(drawData, uiDrawCommands.data(), sizeof(UIIndirectCommand) * uiDrawCommands.size());

	auto uiDrawDataBuffer = createBuffer("uiDrawDataBuffer", sizeof(UIDrawData) * uiDrawData.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	void* td = uiDrawDataBuffer.allocation->GetMappedData();
	memcpy(td, uiDrawData.data(), sizeof(UIDrawData) * uiDrawData.size());

	auto uiMaterialDataBuffer = createBuffer("uiMaterialDataBuffer", sizeof(UIMaterialData) * uiMaterialData.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
	void* md = uiMaterialDataBuffer.allocation->GetMappedData();
	memcpy(md, uiMaterialData.data(), sizeof(UIMaterialData) * uiMaterialData.size());

	uiDrawBatch.commands = {
		.buffer = uiDrawCommandsBuffer,
		.offset = offsetof(UIIndirectCommand, command),
		.size = static_cast<uint32_t>(uiDrawCommands.size()),
		.stride = sizeof(UIIndirectCommand)
	};

	std::vector<DrawBatchDescriptor> descriptors;
	descriptors.emplace_back(0, uiUniformBuffer, sizeof(UIUniformData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	descriptors.emplace_back(1, uiDrawCommandsBuffer, sizeof(UIIndirectCommand) * uiDrawCommands.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	descriptors.emplace_back(2, uiDrawDataBuffer, sizeof(UIDrawData) * uiDrawData.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	descriptors.emplace_back(3, uiMaterialDataBuffer, sizeof(UIMaterialData) * uiMaterialData.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	uiDrawBatch.descriptors = descriptors;
	uiDrawBatch.descriptorSetLayout = uiDescriptorLayout;

	getCurrentFrame().uiDrawBatches.push_back(uiDrawBatch);

	// text
	if (textDrawCommands.size() > 0) {
		auto textDrawCommandsBuffer = createBuffer("textIndirectCommandBuffer", sizeof(UIIndirectCommand) * textDrawCommands.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* tdd = textDrawCommandsBuffer.allocation->GetMappedData();
		memcpy(tdd, textDrawCommands.data(), sizeof(UIIndirectCommand) * textDrawCommands.size());

		auto textTransformDataBuffer = createBuffer("textTransformBuffer", sizeof(glm::mat4) * textTransformData.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* ttd = textTransformDataBuffer.allocation->GetMappedData();
		memcpy(ttd, textTransformData.data(), sizeof(glm::mat4) * textTransformData.size());

		textDrawBatch.commands = {
			.buffer = textDrawCommandsBuffer,
			.offset = offsetof(UIIndirectCommand, command),
			.size = static_cast<uint32_t>(textDrawCommands.size()),
			.stride = sizeof(UIIndirectCommand)
		};

		std::vector<DrawBatchDescriptor> textDescriptors;
		textDescriptors.emplace_back(0, fontUniformBuffer, sizeof(FontUniformData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		textDescriptors.emplace_back(2, textDrawCommandsBuffer, sizeof(UIIndirectCommand) * textDrawCommands.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		textDescriptors.emplace_back(3, textTransformDataBuffer, sizeof(glm::mat4) * textTransformData.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		std::vector<DrawBatchImageDescriptor> textImageDescriptors;
		textImageDescriptors.emplace_back(1, fontSDF.view, fontSDF.sampler, fontSDF.imageLayout, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

		textDrawBatch.descriptors = textDescriptors;
		textDrawBatch.imageDescriptors = textImageDescriptors;
		textDrawBatch.descriptorSetLayout = fontDescriptorLayout;

		getCurrentFrame().uiDrawBatches.push_back(textDrawBatch);

		getCurrentFrame().m_deletionQueue.push([this, textDrawCommandsBuffer, textTransformDataBuffer]() {
			destroyBuffer(textDrawCommandsBuffer);
			destroyBuffer(textTransformDataBuffer);
		});
	}

	getCurrentFrame().m_deletionQueue.push([this, uiDrawCommandsBuffer, uiDrawDataBuffer, uiMaterialDataBuffer, uiGeometryBuffers]() {
		destroyBuffer(uiDrawCommandsBuffer);
		destroyBuffer(uiDrawDataBuffer);
		destroyBuffer(uiMaterialDataBuffer);
		destroyBuffer(uiGeometryBuffers.vertexBuffer);
		destroyBuffer(uiGeometryBuffers.indexBuffer);
	});

	auto genTime = std::chrono::duration<double, std::micro>(std::chrono::system_clock::now() - start).count();

	m_rendererState->rendererStats.uiDrawBatchGenerationTimeAvg = m_rendererState->rendererStats.uiDrawBatchGenerationTimeAvg * 0.95 + genTime * 0.05;
}

void VulkanRenderer::buildDrawBatches(std::vector<Model*>& models) {
	getCurrentFrame().drawBatches.clear();


	// TODO: Right now each model use their own vertex and index buffers
	// 			 This might not be efficient since we can, very likely,
	// 			 combine multiple models and render them in the same draw batches
	// 			 if they use the same vertex/index buffers.
	double flattenTime{};
	double genTime{};
	for (auto model : models) {
		std::vector<Entity*> entities{};
		auto start = std::chrono::high_resolution_clock::now();
		Entity_flattenHierarchy(model->root, glm::mat4{ 1.0f }, entities);
		flattenTime += std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - start).count();

		std::vector<DrawBatch> drawBatches{};

		auto startGen = std::chrono::high_resolution_clock::now();
		DrawBatch opaque{};
		opaque.meshBuffers = model->modelBuffers;
		opaque.pipeline = opaquePipeline.pipeline;
		opaque.pipelineLayout = opaquePipeline.layout;

		DrawBatch transparent{};
		transparent.meshBuffers = model->modelBuffers;
		transparent.pipeline = transparentPipeline.pipeline;
		transparent.pipelineLayout = transparentPipeline.layout;

		DrawBatch doubleSided{};
		doubleSided.meshBuffers = model->modelBuffers;
		doubleSided.pipeline = doubleSidedPipeline.pipeline;
		doubleSided.pipelineLayout = doubleSidedPipeline.layout;

		auto opaqueCommands = std::vector<MeshIndirectCommand>();
		auto doubleSidedCommands = std::vector<MeshIndirectCommand>();
		auto transparentCommands = std::vector<MeshIndirectCommand>();

		std::vector<MeshDraw> opaqueDraws{};
		std::vector<MeshDraw> doubleSidedDraws{};
		std::vector<MeshDraw> transparentDraws{};

		for (auto& entity : entities) {
			if (entity->mesh == nullptr) {
				continue;
			}
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
		}

		auto meshIndirectDoubleSidedCommandsBuffer = createBuffer("meshDrawCommandsBuffer DoubleSided", sizeof(MeshIndirectCommand) * doubleSidedCommands.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* uiData1 = meshIndirectDoubleSidedCommandsBuffer.allocation->GetMappedData();
		memcpy(uiData1, doubleSidedCommands.data(), sizeof(MeshIndirectCommand) * doubleSidedCommands.size());

		auto meshIndirectTransparentCommandsBuffer = createBuffer("meshDrawCommandsBuffer Transparent", sizeof(MeshIndirectCommand) * transparentCommands.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* uiData2 = meshIndirectTransparentCommandsBuffer.allocation->GetMappedData();
		memcpy(uiData2, transparentCommands.data(), sizeof(MeshIndirectCommand) * transparentCommands.size());

		auto doubleSidedDrawsDataBuffer = createBuffer("meshTransformBuffer Opaque", sizeof(MeshDraw) * doubleSidedDraws.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* mtd1 = doubleSidedDrawsDataBuffer.allocation->GetMappedData();
		memcpy(mtd1, doubleSidedDraws.data(), sizeof(MeshDraw) * doubleSidedDraws.size());

		auto transparentDrawsDataBuffer = createBuffer("meshTransformBuffer Transparent", sizeof(MeshDraw) * transparentDraws.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* mtd2 = transparentDrawsDataBuffer.allocation->GetMappedData();
		memcpy(mtd2, transparentDraws.data(), sizeof(MeshDraw) * transparentDraws.size());

		std::vector<DrawBatchDescriptor> doubleSidedDescriptors;
		doubleSidedDescriptors.emplace_back(0, globalMaterialDataBuffer, sizeof(MaterialData) * MaterialCache_size(m_materialCache), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		doubleSidedDescriptors.emplace_back(1, meshIndirectDoubleSidedCommandsBuffer, sizeof(MeshIndirectCommand) * doubleSidedCommands.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		doubleSidedDescriptors.emplace_back(2, doubleSidedDrawsDataBuffer, sizeof(MeshDraw) * doubleSidedDraws.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		std::vector<DrawBatchDescriptor> transparentDescriptors;
		transparentDescriptors.emplace_back(0, globalMaterialDataBuffer, sizeof(MaterialData) * MaterialCache_size(m_materialCache), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		transparentDescriptors.emplace_back(1, meshIndirectTransparentCommandsBuffer, sizeof(MeshIndirectCommand) * transparentCommands.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
		transparentDescriptors.emplace_back(2, transparentDrawsDataBuffer, sizeof(MeshDraw) * transparentDraws.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

		transparent.commands = {
			.buffer = meshIndirectTransparentCommandsBuffer,
			.offset = offsetof(MeshIndirectCommand, command),
			.size = static_cast<uint32_t>(transparentCommands.size()),
			.stride = sizeof(MeshIndirectCommand)
		};
		transparent.descriptors = transparentDescriptors;
		transparent.descriptorSetLayout = m_modelDrawDescriptorLayout;

		doubleSided.commands = {
			.buffer = meshIndirectDoubleSidedCommandsBuffer,
			.offset = offsetof(MeshIndirectCommand, command),
			.size = static_cast<uint32_t>(doubleSidedCommands.size()),
			.stride = sizeof(MeshIndirectCommand)
		};
		doubleSided.descriptors = doubleSidedDescriptors;
		doubleSided.descriptorSetLayout = m_modelDrawDescriptorLayout;

		if (opaqueCommands.size() > 0) {
			auto meshIndirectOpaqueCommandsBuffer = createBuffer("meshDrawCommandsBuffer Opaque", sizeof(MeshIndirectCommand) * opaqueCommands.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			void* uiData = meshIndirectOpaqueCommandsBuffer.allocation->GetMappedData();
			memcpy(uiData, opaqueCommands.data(), sizeof(MeshIndirectCommand) * opaqueCommands.size());

			auto opaqueDrawsDataBuffer = createBuffer("meshTransformBuffer Opaque", sizeof(MeshDraw) * opaqueDraws.size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			void* mtd = opaqueDrawsDataBuffer.allocation->GetMappedData();
			memcpy(mtd, opaqueDraws.data(), sizeof(MeshDraw) * opaqueDraws.size());

			std::vector<DrawBatchDescriptor> opaqueDescriptors;
			opaqueDescriptors.emplace_back(0, globalMaterialDataBuffer, sizeof(MaterialData) * MaterialCache_size(m_materialCache), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			opaqueDescriptors.emplace_back(1, meshIndirectOpaqueCommandsBuffer, sizeof(MeshIndirectCommand) * opaqueCommands.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			opaqueDescriptors.emplace_back(2, opaqueDrawsDataBuffer, sizeof(MeshDraw) * opaqueDraws.size(), 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

			opaque.commands = {
				.buffer = meshIndirectOpaqueCommandsBuffer,
				.offset = offsetof(MeshIndirectCommand, command),
				.size = static_cast<uint32_t>(opaqueCommands.size()),
				.stride = sizeof(MeshIndirectCommand)
			};
			opaque.descriptors = opaqueDescriptors;
			opaque.descriptorSetLayout = m_modelDrawDescriptorLayout;

			getCurrentFrame().drawBatches.push_back(opaque);

			getCurrentFrame().m_deletionQueue.push([this, opaqueDrawsDataBuffer, meshIndirectOpaqueCommandsBuffer]() {
				destroyBuffer(opaqueDrawsDataBuffer);
				destroyBuffer(meshIndirectOpaqueCommandsBuffer);
			});
		}

		getCurrentFrame().drawBatches.push_back(doubleSided);
		getCurrentFrame().drawBatches.push_back(transparent);

		getCurrentFrame().m_deletionQueue.push([this, doubleSidedDrawsDataBuffer, transparentDrawsDataBuffer, meshIndirectDoubleSidedCommandsBuffer, meshIndirectTransparentCommandsBuffer]() {
			destroyBuffer(doubleSidedDrawsDataBuffer);
			destroyBuffer(transparentDrawsDataBuffer);
			destroyBuffer(meshIndirectDoubleSidedCommandsBuffer);
			destroyBuffer(meshIndirectTransparentCommandsBuffer);
		});
		genTime += std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - start).count();
	}

	m_rendererState->rendererStats.entityFlattenTimeAvg = m_rendererState->rendererStats.entityFlattenTimeAvg * 0.95 + flattenTime * 0.05;
	m_rendererState->rendererStats.drawBatchGenerationTimeAvg = m_rendererState->rendererStats.drawBatchGenerationTimeAvg * 0.95 + genTime * 0.05;
}


void VulkanRenderer::draw(float deltaTime) {
	updateScene(deltaTime);
	updateUIData();
	updateFontData();

	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(m_device, 1, &getCurrentFrame().m_renderFence, true, 1000000000));

	getCurrentFrame().m_deletionQueue.flush();
	getCurrentFrame().m_frameDescriptors.clearPools(m_device);

	uint32_t swapchainImageIndex{};
	VkResult e = vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, getCurrentFrame().m_swapchainSemaphore, nullptr, &swapchainImageIndex);
	if (e == VK_ERROR_OUT_OF_DATE_KHR) {
		m_rendererState->resizeRequested = true;
		return;
	}

	VK_CHECK(vkResetFences(m_device, 1, &getCurrentFrame().m_renderFence));

	auto commandBuffer = getCurrentFrame().m_commandBuffer;

	VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

	std::vector<Model*> modelsToRender{};
	modelsToRender.push_back(&loadedModels["testModel"]);
	buildDrawBatches(modelsToRender);

	auto commandBeginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	m_drawExtent.width = std::min(m_swapchainExtent.width, m_drawImage.imageExtent.width) * m_renderScale;
	m_drawExtent.height = std::min(m_swapchainExtent.height, m_drawImage.imageExtent.height) * m_renderScale;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBeginInfo));

	vkCmdResetQueryPool(commandBuffer, timestampPool, 0, 128);
	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestampPool, 0);

	// transition our main draw image into general layout so we can write into it
	// we will overwrite it all so we dont care about what was the older layout
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	drawBackground(commandBuffer);

	// transition the draw image and the depth image into their correct attachment layouts
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	transitionImage(commandBuffer, m_depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	vkCmdResetQueryPool(commandBuffer, pipelineStatisticsPool, 0, 1);
	vkCmdBeginQuery(commandBuffer, pipelineStatisticsPool, 0, 0);

	drawGeometry(commandBuffer);

	vkCmdEndQuery(commandBuffer, pipelineStatisticsPool, 0);

	// transition the draw image and the swapchain image into their correct transfer layouts
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	transitionImage(commandBuffer, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	copyImageToImage(commandBuffer, m_drawImage.image, m_swapchainImages[swapchainImageIndex], m_drawExtent, m_swapchainExtent);

	// set swapchain image layout to Present so we can show it on the screen
	transitionImage(commandBuffer, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);


	vkCmdWriteTimestamp(commandBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestampPool, 1);

	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	// prepare the submission to the queue.
	// we want to wait on the m_presentSemaphore, as that semaphore is signaled when the swapchain is ready
	// we will signal the m_renderSemaphore, to signal that rendering has finished
	auto cmdinfo = commandBufferSubmitInfo(commandBuffer);

	auto waitInfo = semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, getCurrentFrame().m_swapchainSemaphore);
	auto signalInfo = semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame().m_renderSemaphore);

	VkSubmitInfo2 submit = submitInfo(&cmdinfo, &signalInfo, &waitInfo);

	// submit command buffer to the queue and execute it.
	// m_renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(m_graphicsQueue, 1, &submit, getCurrentFrame().m_renderFence));

	// prepare present
	// this will put the image we just rendered to into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as its necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &getCurrentFrame().m_renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VkResult presentResult = vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
	if (presentResult == VK_ERROR_OUT_OF_DATE_KHR) {
		m_rendererState->resizeRequested = true;
	}

	// TODO: we are waiting on fences twice inside this function. Need to rework this logic.
	// This is just here so we can query timestamp results.
	VK_CHECK(vkWaitForFences(m_device, 1, &getCurrentFrame().m_renderFence, VK_TRUE, ~0ull));

	// Get timestamp results
	std::array<uint64_t, 2> timestampResults{};
	VK_CHECK(vkGetQueryPoolResults(m_device, timestampPool, 0, timestampResults.size(), timestampResults.size() * sizeof(uint64_t), timestampResults.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT));

	std::array<uint64_t, 1> pipelineStatisticsResults{};
	VK_CHECK(vkGetQueryPoolResults(m_device, pipelineStatisticsPool, 0, pipelineStatisticsResults.size(), pipelineStatisticsResults.size() * sizeof(uint64_t), pipelineStatisticsResults.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT));

	double frameGpuBegin = double(timestampResults[0]) * physicalDeviceTimestampPeriod * 1e-6;
	double frameGpuEnd = double(timestampResults[1]) * physicalDeviceTimestampPeriod * 1e-6;
	m_rendererState->rendererStats.frameGpuTimeAvg = m_rendererState->rendererStats.frameGpuTimeAvg * 0.95 + (frameGpuEnd - frameGpuBegin) * 0.05;
	m_rendererState->rendererStats.triangleCount = pipelineStatisticsResults[0];

	// move to the next frame.
	m_frameNumber++;
}

void VulkanRenderer::drawBackground(VkCommandBuffer commandBuffer) {
	ComputePushConstants data = {
		.data1 = { 0.1, 0.2, 0.4, 0.97 }
	};
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_skyPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_skyPipelineLayout, 0, 1, &m_drawImageDescriptors, 0, nullptr);
	vkCmdPushConstants(commandBuffer, m_skyPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &data);
	vkCmdDispatch(commandBuffer, std::ceil(m_drawExtent.width / 16.0), std::ceil(m_drawExtent.height / 16.0), 1);
}

void VulkanRenderer::drawGeometry(VkCommandBuffer commandBuffer) {
	m_rendererState->rendererStats.drawCallCount = 0;

	auto start = std::chrono::system_clock::now();

	VkRenderingAttachmentInfo colorAttachment = attachmentInfo(m_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingAttachmentInfo depthAttachment = depthAttachmentInfo(m_depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = renderingInfo(m_drawExtent, &colorAttachment, &depthAttachment);
	vkCmdBeginRendering(commandBuffer, &renderInfo);

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = static_cast<float>(m_drawExtent.width);
	viewport.height = static_cast<float>(m_drawExtent.height);
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = m_drawExtent.width;
	scissor.extent.height = m_drawExtent.height;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// allocate a new uniform buffer for the scene data
	// This should be deleted each frame.
	AllocatedBuffer gpuSceneDataBuffer = createBuffer("gpuSceneDataBuffer", sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// write the buffer
	auto* sceneUniformData = static_cast<GPUSceneData*>(gpuSceneDataBuffer.allocation->GetMappedData());
	*sceneUniformData = m_sceneData;

	// Create global sceneData descriptor
	VkDescriptorSet globalDescriptor = getCurrentFrame().m_frameDescriptors.allocate(m_device, m_gpuSceneDataDescriptorLayout);

	getCurrentFrame().m_deletionQueue.push([gpuSceneDataBuffer, this]() {
		destroyBuffer(gpuSceneDataBuffer);
	});

	{
		DescriptorWriter writer;
		writer.writeBuffer(0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		writer.updateSet(m_device, globalDescriptor);
	}

	uint32_t drawCallCount{};

	// Process this frames draw batches
	for (auto& drawBatch : getCurrentFrame().drawBatches) {
		drawCallCount += drawBatch.commands.size;

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawBatch.pipeline);

		VkDescriptorSet drawBatchDescriptor = getCurrentFrame().m_frameDescriptors.allocate(m_device, drawBatch.descriptorSetLayout);
		DescriptorWriter drawBatchDescriptorWriter;
		for (auto& descriptor : drawBatch.descriptors) {
			drawBatchDescriptorWriter.writeBuffer(descriptor.binding, descriptor.buffer.buffer, descriptor.size, descriptor.offset, descriptor.type);
		}
		drawBatchDescriptorWriter.updateSet(m_device, drawBatchDescriptor);

		// Global descriptor sets. Bindings 0 and 1 are reserved.
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawBatch.pipelineLayout, 0, 1, &globalDescriptor, 0, nullptr);
		// TODO: This might not need to be global.
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawBatch.pipelineLayout, 1, 1, &bindlessTexturesDescriptorSet, 0, nullptr);

		// Draw batch specific descriptor set
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawBatch.pipelineLayout, 2, 1, &drawBatchDescriptor, 0, nullptr);

		// TODO: Should be included as part of a Batch
		GPUDrawPushConstants pushConstants{};
		pushConstants.vertexBuffer = drawBatch.meshBuffers.vertexBufferAddress;

		vkCmdBindIndexBuffer(commandBuffer, drawBatch.meshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(commandBuffer, drawBatch.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
		vkCmdDrawIndexedIndirect(commandBuffer, drawBatch.commands.buffer.buffer, drawBatch.commands.offset, drawBatch.commands.size, drawBatch.commands.stride);
	}

	m_rendererState->rendererStats.drawCallCount = static_cast<int32_t>(drawCallCount);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

	// UI rendering

	buildUIDrawBatches(getCurrentFrame().uiRenderCommands);

	auto uiStart = std::chrono::system_clock::now();
	for (auto& drawBatch : getCurrentFrame().uiDrawBatches) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawBatch.pipeline);

		VkDescriptorSet drawBatchDescriptor = getCurrentFrame().m_frameDescriptors.allocate(m_device, drawBatch.descriptorSetLayout);
		DescriptorWriter drawBatchDescriptorWriter;
		for (auto& descriptor : drawBatch.descriptors) {
			drawBatchDescriptorWriter.writeBuffer(descriptor.binding, descriptor.buffer.buffer, descriptor.size, descriptor.offset, descriptor.type);
		}
		for (auto& descriptor : drawBatch.imageDescriptors) {
			drawBatchDescriptorWriter.writeImage(descriptor.binding, descriptor.imageView, descriptor.sampler, descriptor.imageLayout, descriptor.type);
		}
		drawBatchDescriptorWriter.updateSet(m_device, drawBatchDescriptor);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawBatch.pipelineLayout, 0, 1, &drawBatchDescriptor, 0, nullptr);

		// TODO: Should be included as part of a Batch
		UIPushConstants uiPushConstants{};
		uiPushConstants.vertexBuffer = drawBatch.meshBuffers.vertexBufferAddress;

		vkCmdBindIndexBuffer(commandBuffer, drawBatch.meshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(commandBuffer, drawBatch.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(UIPushConstants), &uiPushConstants);
		vkCmdDrawIndexedIndirect(commandBuffer, drawBatch.commands.buffer.buffer, drawBatch.commands.offset, drawBatch.commands.size, drawBatch.commands.stride);
	}

	auto uiEnd = std::chrono::system_clock::now();
	auto uiElapsed = std::chrono::duration_cast<std::chrono::microseconds>(uiEnd - uiStart);

	vkCmdEndRendering(commandBuffer);

	m_rendererState->rendererStats.meshDrawTimeAvg = m_rendererState->rendererStats.meshDrawTimeAvg * 0.95 + (static_cast<float>(elapsed.count()) / 1000.0f) * 0.05;
	m_rendererState->rendererStats.uiFrametimeAvg = m_rendererState->rendererStats.uiFrametimeAvg * 0.95 + (static_cast<float>(uiElapsed.count()) / 1000.0f) * 0.05;
}

void VulkanRenderer::initDescriptors() {
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 1 }
	};

	m_globalDescriptorAllocator.init(m_device, 10, sizes);
	m_mainDeletionQueue.push([&]() {
		m_globalDescriptorAllocator.destroyPools(m_device);
	});

	// Compute stage image descriptor
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		m_drawImageDescriptorLayout = builder.build(m_device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	m_drawImageDescriptors = m_globalDescriptorAllocator.allocate(m_device, m_drawImageDescriptorLayout);

	{
		DescriptorWriter writer;
		writer.writeImage(0, m_drawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writer.updateSet(m_device, m_drawImageDescriptors);
	}

	// Vertex/Fragment UBO
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		m_gpuSceneDataDescriptorLayout = builder.build(m_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	// Model Draw SSBO
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		m_modelDrawDescriptorLayout = builder.build(m_device);
	}

	m_mainDeletionQueue.push([&]() {
		vkDestroyDescriptorSetLayout(m_device, m_drawImageDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_device, m_gpuSceneDataDescriptorLayout, nullptr);
		vkDestroyDescriptorSetLayout(m_device, m_modelDrawDescriptorLayout, nullptr);
	});

	for (auto& frame : m_frames) {
		// create a descriptor pool
		std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes = {
			{ .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .ratio = 3 },
			{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .ratio = 3 },
			{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 3 },
			{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .ratio = 4 },
		};

		frame.m_frameDescriptors = {};
		frame.m_frameDescriptors.init(m_device, 1000, frame_sizes);

		m_mainDeletionQueue.push([&]() {
			frame.m_frameDescriptors.destroyPools(m_device);
		});
	}

	// Font descriptors
	std::vector<DescriptorAllocator::PoolSizeRatio> fontPoolSizes = {
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 4 },
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .ratio = 2 },
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .ratio = 2 }
	};
	fontDescriptorAllocator.init(m_device, 4, fontPoolSizes);
	m_mainDeletionQueue.push([&]() {
		fontDescriptorAllocator.destroyPools(m_device);
	});

	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		fontDescriptorLayout = builder.build(m_device);
	}

	m_mainDeletionQueue.push([&]() {
		vkDestroyDescriptorSetLayout(m_device, fontDescriptorLayout, nullptr);
	});

	fontDescriptorSet = fontDescriptorAllocator.allocate(m_device, fontDescriptorLayout);

	// UI Descriptors
	std::vector<DescriptorAllocator::PoolSizeRatio> uiPoolSizes = {
		{ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .ratio = 2 },
		{ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .ratio = 2 }
	};
	uiDescriptorAllocator.init(m_device, 2, uiPoolSizes);
	m_mainDeletionQueue.push([&]() {
		uiDescriptorAllocator.destroyPools(m_device);
	});

	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		builder.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
		builder.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT);
		uiDescriptorLayout = builder.build(m_device);
	}

	m_mainDeletionQueue.push([&]() {
		vkDestroyDescriptorSetLayout(m_device, uiDescriptorLayout, nullptr);
	});

	uiDescriptorSet = uiDescriptorAllocator.allocate(m_device, uiDescriptorLayout);
}

void VulkanRenderer::initPipelines() {
	initBackgroundPipelines();
	buildDefaultPipelines();
	initUIPipeline();
	initFontPipeline();
}

void VulkanRenderer::initBackgroundPipelines() {
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &m_drawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VkPushConstantRange pushConstants{};
	pushConstants.offset = 0;
	pushConstants.size = sizeof(ComputePushConstants);
	pushConstants.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

	computeLayout.pPushConstantRanges = &pushConstants;
	computeLayout.pushConstantRangeCount = 1;

	VK_CHECK(vkCreatePipelineLayout(m_device, &computeLayout, nullptr, &m_skyPipelineLayout));

	VkShaderModule computeDrawShader{};
	if (!loadShaderModule("res/shaders/gradient.comp.spv", m_device, &computeDrawShader)) {
		std::cout << std::format("Error when building the compute shader \n");
	}

	auto stageinfo = pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = m_skyPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_skyPipeline));

	vkDestroyShaderModule(m_device, computeDrawShader, nullptr);

	m_mainDeletionQueue.push([this]() {
		vkDestroyPipelineLayout(m_device, m_skyPipelineLayout, nullptr);
		vkDestroyPipeline(m_device, m_skyPipeline, nullptr);
	});
}

void VulkanRenderer::initUIPipeline() {
	VkShaderModule uiFragShader{};
	if (!loadShaderModule("res/shaders/ui.frag.spv", m_device, &uiFragShader)) {
		std::cout << std::format("Error when building the UI fragment shader module\n");
	}

	VkShaderModule uiVertexShader{};
	if (!loadShaderModule("res/shaders/ui.vert.spv", m_device, &uiVertexShader)) {
		std::cout << std::format("Error when building the UI vertex shader module\n");
	}

	VkDescriptorSetLayout layouts[] = {
		uiDescriptorLayout
	};

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(UIPushConstants);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo uiLayoutInfo = pipelineLayoutCreateInfo();
	uiLayoutInfo.pPushConstantRanges = &pushConstantRange;
	uiLayoutInfo.pushConstantRangeCount = 1;
	uiLayoutInfo.setLayoutCount = 1;
	uiLayoutInfo.pSetLayouts = layouts;

	VK_CHECK(vkCreatePipelineLayout(m_device, &uiLayoutInfo, nullptr, &uiPipelineLayout));

	m_mainDeletionQueue.push([&]() {
		vkDestroyPipelineLayout(m_device, uiPipelineLayout, nullptr);
	});

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.setPipelineLayout(uiPipelineLayout);
	pipelineBuilder.setShaders(uiVertexShader, uiFragShader);
	pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	pipelineBuilder.setMultisamplingNone();
	pipelineBuilder.enableBackgroundBlending();
	pipelineBuilder.disableDepthTest();

	pipelineBuilder.setColorAttachmentFormat(m_drawImage.imageFormat);
	pipelineBuilder.setDepthFormat(m_depthImage.imageFormat);

	uiPipeline = pipelineBuilder.buildPipeline(m_device);

	m_mainDeletionQueue.push([&]() {
		vkDestroyPipeline(m_device, uiPipeline, nullptr);
	});

	vkDestroyShaderModule(m_device, uiFragShader, nullptr);
	vkDestroyShaderModule(m_device, uiVertexShader, nullptr);
}

void VulkanRenderer::initFontPipeline() {
	VkShaderModule fontFragShader{};
	if (!loadShaderModule("res/shaders/sdf_text.frag.spv", m_device, &fontFragShader)) {
		std::cout << std::format("Error when building the font fragment shader module") << '\n';
	}

	VkShaderModule fontVertexShader{};
	if (!loadShaderModule("res/shaders/sdf_text.vert.spv", m_device, &fontVertexShader)) {
		std::cout << std::format("Error when building the font vertex shader module") << '\n';
	}

	VkDescriptorSetLayout layouts[] = {
		fontDescriptorLayout
	};

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(UIPushConstants);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo fontLayoutInfo = pipelineLayoutCreateInfo();
	fontLayoutInfo.pPushConstantRanges = &pushConstantRange;
	fontLayoutInfo.pushConstantRangeCount = 1;
	fontLayoutInfo.setLayoutCount = 1;
	fontLayoutInfo.pSetLayouts = layouts;

	VK_CHECK(vkCreatePipelineLayout(m_device, &fontLayoutInfo, nullptr, &fontPipelineLayout));

	m_mainDeletionQueue.push([&]() {
		vkDestroyPipelineLayout(m_device, fontPipelineLayout, nullptr);
	});

	PipelineBuilder pipelineBuilder;
	pipelineBuilder.setPipelineLayout(fontPipelineLayout);
	pipelineBuilder.setShaders(fontVertexShader, fontFragShader);
	pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	pipelineBuilder.setMultisamplingNone();
	pipelineBuilder.enableBackgroundBlending();
	pipelineBuilder.disableDepthTest();

	pipelineBuilder.setColorAttachmentFormat(m_drawImage.imageFormat);
	pipelineBuilder.setDepthFormat(m_depthImage.imageFormat);

	fontPipeline = pipelineBuilder.buildPipeline(m_device);

	m_mainDeletionQueue.push([&]() {
		vkDestroyPipeline(m_device, fontPipeline, nullptr);
	});

	vkDestroyShaderModule(m_device, fontFragShader, nullptr);
	vkDestroyShaderModule(m_device, fontVertexShader, nullptr);
}

void VulkanRenderer::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
	VK_CHECK(vkResetFences(m_device, 1, &m_immFence));
	VK_CHECK(vkResetCommandBuffer(m_immCommandBuffer, 0));

	VkCommandBuffer cmd = m_immCommandBuffer;

	VkCommandBufferBeginInfo cmdBeginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkCommandBufferSubmitInfo cmdinfo = commandBufferSubmitInfo(cmd);
	VkSubmitInfo2 submit = submitInfo(&cmdinfo, nullptr, nullptr);

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit2(m_graphicsQueue, 1, &submit, m_immFence));

	VK_CHECK(vkWaitForFences(m_device, 1, &m_immFence, true, 9999999999));
}

AllocatedBuffer VulkanRenderer::createBuffer(std::string name, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
	// allocate buffer
	VkBufferCreateInfo bufferInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	AllocatedBuffer newBuffer{};

	// allocate the buffer
	VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation, &newBuffer.info));

	vmaSetAllocationName(m_allocator, newBuffer.allocation, name.c_str());

	return newBuffer;
}

void VulkanRenderer::destroyBuffer(const AllocatedBuffer& buffer) {
	vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
}

/*
 * Create a staging buffer in CPU memory to hold the vertex + index buffer data.
 * Copy it to the GPU buffer.
 */
template<typename VertexType>
GPUMeshBuffers VulkanRenderer::uploadMesh(std::span<uint32_t> indices, std::span<VertexType> vertices, std::string name) {
	const auto vertexBufferSize = vertices.size() * sizeof(VertexType);
	const auto indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface{};

	// create vertex buffer
	newSurface.vertexBuffer = createBuffer(name + " MeshVertexBuffer", vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	// find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer
	};
	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_device, &deviceAdressInfo);

	// create index buffer
	newSurface.indexBuffer = createBuffer(name + " MeshIndexBuffer", indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	AllocatedBuffer staging = createBuffer(name + " Mesh staging", vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	// Get a pointer to which we can write to
	void* data = staging.allocation->GetMappedData();

	// copy vertex buffer
	memcpy(data, vertices.data(), vertexBufferSize);
	// copy index buffer
	memcpy(static_cast<char*>(data) + vertexBufferSize, indices.data(), indexBufferSize);

	immediateSubmit([&](VkCommandBuffer cmd) {
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

	destroyBuffer(staging);

	return newSurface;
}


AllocatedImage VulkanRenderer::createImage(std::string name, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
	AllocatedImage newImage{};
	newImage.imageFormat = format;
	newImage.imageExtent = size;

	VkImageCreateInfo img_info = imageCreateInfo(format, usage, size);
	if (mipmapped) {
		img_info.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(size.width, size.height)))) + 1;
	}

	// always allocate images on dedicated GPU memory
	VmaAllocationCreateInfo allocinfo = {};
	allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	VK_CHECK(vmaCreateImage(m_allocator, &img_info, &allocinfo, &newImage.image, &newImage.allocation, nullptr));

	vmaSetAllocationName(m_allocator, newImage.allocation, name.c_str());

	// if the format is a depth format, we will need to have it use the correct
	// aspect flag
	VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format == VK_FORMAT_D32_SFLOAT) {
		aspectFlag = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	// build a image-view for the image
	VkImageViewCreateInfo view_info = imageViewCreateInfo(format, newImage.image, aspectFlag);
	view_info.subresourceRange.levelCount = img_info.mipLevels;

	VK_CHECK(vkCreateImageView(m_device, &view_info, nullptr, &newImage.imageView));

	return newImage;
}

AllocatedImage VulkanRenderer::createImage(std::string name, void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
	size_t data_size = size.depth * size.width * size.height * 4;
	AllocatedBuffer uploadbuffer = createBuffer("createImage uploadBuffer", data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(uploadbuffer.info.pMappedData, data, data_size);

	AllocatedImage newImage = createImage(name, size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

	immediateSubmit([&](VkCommandBuffer cmd) {
		transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = size;

		// copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
		if (mipmapped) {
			generateMipmaps(cmd, newImage.image, { newImage.imageExtent.width, newImage.imageExtent.height });
		} else {
			transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	});

	destroyBuffer(uploadbuffer);

	return newImage;
}

void VulkanRenderer::destroyImage(const AllocatedImage& img) {
	vkDestroyImageView(m_device, img.imageView, nullptr);
	vmaDestroyImage(m_allocator, img.image, img.allocation);
}

void VulkanRenderer::initBindlessTextureDescriptor() {
	constexpr uint32_t maxBindlessTextureResources = 1000;

	VkDescriptorPoolSize poolSizesBindless[] = {
		{ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = maxBindlessTextureResources }
	};

	VkDescriptorPoolCreateInfo poolInfo{ .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	poolInfo.maxSets = 1;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = poolSizesBindless;

	VK_CHECK(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &bindlessPool));

	VkDescriptorBindingFlags bindlessFlags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

	VkDescriptorSetLayoutBinding layoutBinding;
	layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	layoutBinding.descriptorCount = maxBindlessTextureResources;
	layoutBinding.binding = 0;
	layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	layoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBindingFlagsCreateInfo extended_info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO, nullptr };
	extended_info.bindingCount = 1;
	extended_info.pBindingFlags = &bindlessFlags;

	VkDescriptorSetLayoutCreateInfo bindlessTextureLayoutInfo = { .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	bindlessTextureLayoutInfo.bindingCount = 1;
	bindlessTextureLayoutInfo.pBindings = &layoutBinding;
	bindlessTextureLayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

	bindlessTextureLayoutInfo.pNext = &extended_info;

	vkCreateDescriptorSetLayout(m_device, &bindlessTextureLayoutInfo, nullptr, &bindlessTexturesSetLayout);

	VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO };
	countInfo.descriptorSetCount = 1;
	countInfo.pDescriptorCounts = &maxBindlessTextureResources;

	VkDescriptorSetAllocateInfo setAllocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	setAllocateInfo.descriptorPool = bindlessPool;
	setAllocateInfo.descriptorSetCount = 1;
	setAllocateInfo.pSetLayouts = &bindlessTexturesSetLayout;

	setAllocateInfo.pNext = &countInfo;

	VK_CHECK(vkAllocateDescriptorSets(m_device, &setAllocateInfo, &bindlessTexturesDescriptorSet));

	m_mainDeletionQueue.push([&]() {
		vkDestroyDescriptorSetLayout(m_device, bindlessTexturesSetLayout, nullptr);
		vkDestroyDescriptorPool(m_device, bindlessPool, nullptr);
	});
}

void VulkanRenderer::buildDefaultPipelines() {
	VkShaderModule meshFragShader{};
	if (!loadShaderModule("res/shaders/mesh.frag.spv", m_device, &meshFragShader)) {
		std::cout << std::format("Error when building the mesh fragment shader module") << '\n';
	}

	VkShaderModule meshVertexShader{};
	if (!loadShaderModule("res/shaders/mesh.vert.spv", m_device, &meshVertexShader)) {
		std::cout << std::format("Error when building the mesh vertex shader module") << '\n';
	}

	VkPushConstantRange matrixRange{};
	matrixRange.offset = 0;
	matrixRange.size = sizeof(GPUDrawPushConstants);
	matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	std::array<VkDescriptorSetLayout, 3> layouts = {
		m_gpuSceneDataDescriptorLayout,
		bindlessTexturesSetLayout,
		m_modelDrawDescriptorLayout
	};

	VkPipelineLayoutCreateInfo meshLayoutInfo = pipelineLayoutCreateInfo();
	meshLayoutInfo.pPushConstantRanges = &matrixRange;
	meshLayoutInfo.pushConstantRangeCount = 1;
	meshLayoutInfo.setLayoutCount = layouts.size();
	meshLayoutInfo.pSetLayouts = layouts.data();

	VkPipelineLayout newLayout{};
	VK_CHECK(vkCreatePipelineLayout(m_device, &meshLayoutInfo, nullptr, &newLayout));

	opaquePipeline.layout = newLayout;
	transparentPipeline.layout = newLayout;
	doubleSidedPipeline.layout = newLayout;

	m_mainDeletionQueue.push([&, newLayout]() {
		vkDestroyPipelineLayout(m_device, newLayout, nullptr);
	});

	// build the stage-create-info for both vertex and fragment stages. This lets
	// the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;
	pipelineBuilder.setPipelineLayout(newLayout);
	pipelineBuilder.setShaders(meshVertexShader, meshFragShader);
	pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.setCullMode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.setMultisamplingNone();
	pipelineBuilder.disableBlending();
	pipelineBuilder.enableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

	// render format
	pipelineBuilder.setColorAttachmentFormat(m_drawImage.imageFormat);
	pipelineBuilder.setDepthFormat(m_depthImage.imageFormat);

	// build opaque pipeline
	opaquePipeline.pipeline = pipelineBuilder.buildPipeline(m_device);

	// create the double sided variant
	pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	doubleSidedPipeline.pipeline = pipelineBuilder.buildPipeline(m_device);

	// create the alpha blending variant
	pipelineBuilder.enableBlendingAlphablend();
	transparentPipeline.pipeline = pipelineBuilder.buildPipeline(m_device);

	m_mainDeletionQueue.push([&] {
		vkDestroyPipeline(m_device, opaquePipeline.pipeline, nullptr);
		vkDestroyPipeline(m_device, doubleSidedPipeline.pipeline, nullptr);
		vkDestroyPipeline(m_device, transparentPipeline.pipeline, nullptr);
	});

	vkDestroyShaderModule(m_device, meshFragShader, nullptr);
	vkDestroyShaderModule(m_device, meshVertexShader, nullptr);
}

void VulkanRenderer::writeBindlessTextureToGlobalDescriptor(VkDescriptorSet bindlessTextureSet, AllocatedImage& image, VkSampler sampler, uint32_t index) {
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageView = image.imageView;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.sampler = sampler;

	VkWriteDescriptorSet write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.dstSet = bindlessTextureSet;
	write.dstBinding = 0;
	write.dstArrayElement = index;
	write.descriptorCount = 1;
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
}

void VulkanRenderer::updateScene(float deltaTime) {
	auto start = std::chrono::system_clock::now();

	m_rendererState->mainCamera->update(deltaTime);

	m_sceneData.view = m_rendererState->mainCamera->getViewMatrix();
	m_sceneData.proj = m_rendererState->mainCamera->getPerspectiveProjection();

	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	m_sceneData.proj[1][1] *= -1;
	m_sceneData.viewproj = m_sceneData.proj * m_sceneData.view;

	// some default lighting parameters
	m_sceneData.ambientColor = glm::vec4(.4f);
	m_sceneData.sunlightColor = glm::vec4(1.f);
	m_sceneData.sunlightDirection = glm::vec4(0, 1, 0.5, 1.f);

	auto end = std::chrono::system_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	m_rendererState->rendererStats.sceneUpdateTimeAvg = m_rendererState->rendererStats.sceneUpdateTimeAvg * 0.95 + (static_cast<float>(elapsed.count()) / 1000.0f) * 0.05;
}

void VulkanRenderer::updateFontData() {
	fontUniformData.outline = 0.0f;

	fontUniformData.view = glm::mat4(1.0f);

	auto w = static_cast<float>(m_rendererState->windowExtent.width);
	auto h = static_cast<float>(m_rendererState->windowExtent.height);
	fontUniformData.projection = glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);

	// copy data into buffer
	void* data = fontUniformBuffer.allocation->GetMappedData();
	memcpy(data, &fontUniformData, sizeof(FontUniformData));
}

void VulkanRenderer::initFontData() {
	// parse font file
	fontChars = parsebmFont("res/fonts/font.fnt");

	// load ktx texture
	fontSDF.loadFromFile("res/fonts/font_sdf_rgba.ktx", VK_FORMAT_R8G8B8A8_UNORM, m_device, m_chosenGPU, getCurrentFrame().m_commandPool, m_graphicsQueue, maxSamplerAnisotropy);

	// Create uniform buffer
	fontUniformBuffer = createBuffer("fontUniformBuffer", sizeof(FontUniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	updateFontData();

	m_mainDeletionQueue.push([&]() {
		destroyBuffer(fontUniformBuffer);
		fontSDF.destroy(m_device);
	});
}

void VulkanRenderer::updateUIData() {
	uiUniformData.view = glm::mat4(1.0f);

	auto w = static_cast<float>(m_rendererState->windowExtent.width);
	auto h = static_cast<float>(m_rendererState->windowExtent.height);
	uiUniformData.projection = glm::ortho(0.0f, w, 0.0f, h, -1.0f, 1.0f);

	void* data = uiUniformBuffer.allocation->GetMappedData();
	memcpy(data, &uiUniformData, sizeof(UIUniformData));

	auto stats = std::format("Frametime: {:.2f}ms | GPU: {:.2f}ms | UI: {:.4f}ms | Triangles: {:.2f}M | DrawCall: {}",
		m_rendererState->rendererStats.frametime,
		m_rendererState->rendererStats.frameGpuTimeAvg,
		m_rendererState->rendererStats.uiFrametimeAvg,
		m_rendererState->rendererStats.triangleCount * 1e-6,
		m_rendererState->rendererStats.drawCallCount);

	auto otherStats = std::format("DrawBatchGen: {:.4f}us | UIDrawBatchGen: {:.4f}us | EntityFlatten: {:.4f}us | UILayout: {:.4f}us | SceneUpdate: {:.4f}us | MeshDraw: {:.4f}us",
		m_rendererState->rendererStats.drawBatchGenerationTimeAvg,
		m_rendererState->rendererStats.uiDrawBatchGenerationTimeAvg,
		m_rendererState->rendererStats.entityFlattenTimeAvg,
		m_rendererState->rendererStats.uiLayoutTimeAvg,
		m_rendererState->rendererStats.sceneUpdateTimeAvg,
		m_rendererState->rendererStats.meshDrawTimeAvg);

	auto posData = std::format("Camera Pos: {:.2f} {:.2f} {:.2f}",
		m_rendererState->mainCamera->position.x,
		m_rendererState->mainCamera->position.y,
		m_rendererState->mainCamera->position.z);

	UI::setFont({ .fontChars = fontChars, .textureWidth = static_cast<float>(fontSDF.width) });

	auto start = std::chrono::high_resolution_clock::now();

	UI::beginLayout();

	UI::openElement();
		UI::pushBox({ .layoutDirection = UILayoutDirection::HORIZONTAL,
				.sizingMode = UISizingMode::FIT,
				.backgroundColor = { 1.0f, 0.0f, 0.0f, 1.0f },
				.padding = 10.0f,
				.childGap = 10.0f });

		UI::openElement();
			UI::pushBox({ .layoutDirection = UILayoutDirection::VERTICAL,
					.sizingMode = UISizingMode::FIT,
					.backgroundColor = { 0.0f, 1.0f, 0.0f, 1.0f },
					.padding = 10.0f,
					.childGap = 10.0f });

			UI::openTextElement();
				UI::pushText({ .text = stats });
			UI::closeTextElement();

			UI::openTextElement();
				UI::pushText({ .text = otherStats });
			UI::closeTextElement();
		UI::closeElement();

		UI::openElement();
			UI::pushBox({ .layoutDirection = UILayoutDirection::VERTICAL, .sizingMode = UISizingMode::FIT, .backgroundColor = { 1.0f, 1.0f, 0.0f, 1.0f }, .padding = 10.0f });

			UI::openTextElement();
				UI::pushText({ .text = "Hello there" });
			UI::closeTextElement();
		UI::closeElement();
	UI::closeElement();

	UI::openElement();
		UI::pushBox({ .layoutDirection = UILayoutDirection::VERTICAL,
				.sizingMode = UISizingMode::FIT,
				.backgroundColor = { 0.0f, 0.0f, 1.0f, 1.0f },
				.padding = 40.0f,
				.childGap = 40.0f });
		UI::openTextElement();
			UI::pushText({ .text = "Test string 90 {}_+-=;'\"\\:,.<>/?" });
		UI::closeTextElement();

		UI::openElement();
			UI::pushBox({ .width = 300.0f, .height = 100.0f, .sizingMode = UISizingMode::STATIC, .backgroundColor = { 0.5f, 0.5f, 0.5f, 1.0f } });
		UI::closeElement();
	UI::closeElement();

	getCurrentFrame().uiRenderCommands = UI::endLayout();

	auto uiLayoutTime = std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - start).count();
	m_rendererState->rendererStats.uiLayoutTimeAvg = m_rendererState->rendererStats.uiLayoutTimeAvg * 0.95 + uiLayoutTime * 0.05;
}

void VulkanRenderer::initUI() {
	UI::initRenderContext({ .width = static_cast<float>(m_rendererState->windowExtent.width), .height = static_cast<float>(m_rendererState->windowExtent.height) });

	uiUniformBuffer = createBuffer("uiUniformBuffer", sizeof(UIUniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

	m_mainDeletionQueue.push([&]() {
		destroyBuffer(uiUniformBuffer);
	});

	updateUIData();
}

// NOTE: I don't like this. Maybe just create 2 specialized functions.
template GPUMeshBuffers VulkanRenderer::uploadMesh<UIVertex>(std::span<uint32_t> indices, std::span<UIVertex> vertices, std::string name);
template GPUMeshBuffers VulkanRenderer::uploadMesh<Vertex>(std::span<uint32_t> indices, std::span<Vertex> vertices, std::string name);


}// namespace pm
