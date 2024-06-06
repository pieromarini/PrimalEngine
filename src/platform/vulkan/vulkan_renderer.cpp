#define VMA_IMPLEMENTATION
#include "vulkan_renderer.h"
#include "platform/vulkan/vulkan_descriptor.h"
#include "platform/vulkan/vulkan_images.h"
#include "platform/vulkan/vulkan_loader.h"
#include "vk_types.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_structures_helpers.h"
#include <SDL3/SDL_vulkan.h>
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
	initDescriptors();
	initPipelines();
	initDefaultData();

	m_rendererState->mainCamera->position = glm::vec3(30.f, -00.f, -085.f);

	std::string structurePath = { "res/models/structure.glb" };
	auto structureFile = loadGltf(this,structurePath);

	assert(structureFile.has_value());

	loadedScenes["structure"] = *structureFile;
}

void VulkanRenderer::resizeSwapchain() {
	vkDeviceWaitIdle(m_device);

	destroySwapchain();

	int w{}, h{};
	SDL_GetWindowSize(m_rendererState->window, &w, &h);
	m_rendererState->windowExtent.width = w;
	m_rendererState->windowExtent.height = h;

	createSwapchain(m_rendererState->windowExtent.width, m_rendererState->windowExtent.height);

	m_rendererState->resizeRequested = false;
}

void VulkanRenderer::initDefaultData() {
	// rectangle
	std::array<Vertex, 4> rect_vertices{};

	rect_vertices[0].position = { 4.0, 0.5, 0 };
	rect_vertices[1].position = { 4.0, 2.0, 0 };
	rect_vertices[2].position = { 1.5, 0.5, 0 };
	rect_vertices[3].position = { 1.5, 2.0, 0 };

	rect_vertices[0].color = { 0.0, 0.0, 0.0, 1.0 };
	rect_vertices[1].color = { 0.5, 0.5, 0.5, 1.0 };
	rect_vertices[2].color = { 1.0, 0.0, 0.0, 1.0 };
	rect_vertices[3].color = { 0.0, 1.0, 0.0, 1.0 };

	rect_vertices[0].uv_x = 1;
	rect_vertices[0].uv_y = 0;
	rect_vertices[1].uv_x = 0;
	rect_vertices[1].uv_y = 0;
	rect_vertices[2].uv_x = 1;
	rect_vertices[2].uv_y = 1;
	rect_vertices[3].uv_x = 0;
	rect_vertices[3].uv_y = 1;

	std::array<uint32_t, 6> rect_indices{};

	rect_indices[0] = 0;
	rect_indices[1] = 1;
	rect_indices[2] = 2;

	rect_indices[3] = 2;
	rect_indices[4] = 1;
	rect_indices[5] = 3;

	rectangle = uploadMesh(rect_indices, rect_vertices);

	// 3 default textures, white, grey, black. 1 pixel each
	uint32_t white = glm::packUnorm4x8(glm::vec4(1, 1, 1, 1));
	whiteImage = createImage((void*)&white, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	uint32_t grey = glm::packUnorm4x8(glm::vec4(0.66f, 0.66f, 0.66f, 1));
	greyImage = createImage((void*)&grey, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	uint32_t black = glm::packUnorm4x8(glm::vec4(0, 0, 0, 0));
	blackImage = createImage((void*)&black, VkExtent3D{ 1, 1, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	// checkerboard image
	uint32_t magenta = glm::packUnorm4x8(glm::vec4(1, 0, 1, 1));
	std::array<uint32_t, 16 * 16> pixels{};// for 16x16 checkerboard texture
	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 16; y++) {
			pixels[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}
	errorCheckerboardImage = createImage(pixels.data(), VkExtent3D{ 16, 16, 1 }, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	VkSamplerCreateInfo sampl = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };

	sampl.magFilter = VK_FILTER_NEAREST;
	sampl.minFilter = VK_FILTER_NEAREST;

	vkCreateSampler(m_device, &sampl, nullptr, &defaultSamplerNearest);

	sampl.magFilter = VK_FILTER_LINEAR;
	sampl.minFilter = VK_FILTER_LINEAR;
	vkCreateSampler(m_device, &sampl, nullptr, &defaultSamplerLinear);

	// Load meshes
	m_testMeshes = loadGltfMeshes(this, "res/models/basicmesh.glb").value();

	GLTFMetallic_Roughness::MaterialResources materialResources{};
	// default the material textures
	materialResources.colorImage = whiteImage;
	materialResources.colorSampler = defaultSamplerLinear;
	materialResources.metalRoughImage = whiteImage;
	materialResources.metalRoughSampler = defaultSamplerLinear;

	// set the uniform buffer for the material data
	AllocatedBuffer materialConstants = createBuffer(sizeof(GLTFMetallic_Roughness::MaterialConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// write the buffer
	GLTFMetallic_Roughness::MaterialConstants* sceneUniformData = (GLTFMetallic_Roughness::MaterialConstants*)materialConstants.allocation->GetMappedData();
	sceneUniformData->colorFactors = glm::vec4{ 1, 1, 1, 1 };
	sceneUniformData->metalRoughFactors = glm::vec4{ 1, 0.5, 0, 0 };

	materialResources.dataBuffer = materialConstants.buffer;
	materialResources.dataBufferOffset = 0;

	defaultData = metalRoughMaterial.writeMaterial(m_device, MaterialPass::MainColor, materialResources, m_globalDescriptorAllocator);

	for (auto& m : m_testMeshes) {
		auto newNode = std::make_shared<MeshNode>();
		newNode->mesh = m;

		newNode->localTransform = glm::mat4{ 1.f };
		newNode->worldTransform = glm::mat4{ 1.f };

		for (auto& s : newNode->mesh->surfaces) {
			s.material = std::make_shared<GLTFMaterial>(defaultData);
		}

		loadedNodes[m->name] = std::move(newNode);
	}
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
	VkPhysicalDeviceVulkan13Features features{};
	features.dynamicRendering = true;
	features.synchronization2 = true;

	// vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{};
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing = true;

	// Use VKBootstrap to select a gpu.
	// We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
	vkb::PhysicalDeviceSelector selector{ vkbInstance };
	vkb::PhysicalDevice physicalDevice = selector
																				 .set_minimum_version(1, 3)
																				 .set_required_features_13(features)
																				 .set_required_features_12(features12)
																				 .set_surface(m_surface)
																				 .select()
																				 .value();

	std::cout << std::format("GPU: {}\n", physicalDevice.name);


	// create the final vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a vulkan application
	m_device = vkbDevice.device;
	m_chosenGPU = physicalDevice.physical_device;

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

	// hardcoding the draw format to 32 bit float
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
}

void VulkanRenderer::initCommands() {
	auto commandPoolInfo = commandPoolCreateInfo(m_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (auto& frame : m_frames) {
		VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &frame.m_commandPool));
		auto commandAllocateInfo = commandBufferAllocateInfo(frame.m_commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(m_device, &commandAllocateInfo, &frame.m_commandBuffer));
	}

	// Create command buffer for immediate submits
	VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_immCommandPool));
	VkCommandBufferAllocateInfo cmdAllocInfo = commandBufferAllocateInfo(m_immCommandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_immCommandBuffer));
}

void VulkanRenderer::initSyncStructures() {
	// create syncronization structures
	// one fence to control when the gpu has finished rendering the frame,
	// and 2 semaphores to syncronize rendering with swapchain
	// we want the fence to start signaled so we can wait on it on the first frame
	auto fenceCreate = fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	auto semaphoreCreate = semaphoreCreateInfo();

	for (auto& m_frame : m_frames) {
		VK_CHECK(vkCreateFence(m_device, &fenceCreate, nullptr, &m_frame.m_renderFence));

		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreate, nullptr, &m_frame.m_swapchainSemaphore));
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreate, nullptr, &m_frame.m_renderSemaphore));
	}
	VK_CHECK(vkCreateFence(m_device, &fenceCreate, nullptr, &m_immFence));
}

void VulkanRenderer::createSwapchain(uint32_t width, uint32_t height) {
	vkb::SwapchainBuilder swapchainBuilder{ m_chosenGPU, m_device, m_surface };

	m_swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain = swapchainBuilder
																	//.use_default_format_selection()
																	.set_desired_format(VkSurfaceFormatKHR{ .format = m_swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
																	// use vsync present mode
																	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
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

 	loadedScenes.clear();

	for (auto& frame : m_frames) {
		vkDestroyCommandPool(m_device, frame.m_commandPool, nullptr);

		vkDestroyFence(m_device, frame.m_renderFence, nullptr);
		vkDestroySemaphore(m_device, frame.m_renderSemaphore, nullptr);
		vkDestroySemaphore(m_device, frame.m_swapchainSemaphore, nullptr);

		frame.m_frameDescriptors.destroyPools(m_device);
	}

	vkDestroyCommandPool(m_device, m_immCommandPool, nullptr);
	vkDestroyFence(m_device, m_immFence, nullptr);

	vkDestroyImageView(m_device, m_drawImage.imageView, nullptr);
	vmaDestroyImage(m_allocator, m_drawImage.image, m_drawImage.allocation);

	vkDestroyImageView(m_device, m_depthImage.imageView, nullptr);
	vmaDestroyImage(m_allocator, m_depthImage.image, m_depthImage.allocation);

	vkDestroyPipelineLayout(m_device, m_gradientPipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_gradientPipeline, nullptr);

	vkDestroyPipelineLayout(m_device, m_meshPipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_meshPipeline, nullptr);

	destroySwapchain();

	vmaDestroyAllocator(m_allocator);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyDevice(m_device, nullptr);

	vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);

	vkDestroyInstance(m_instance, nullptr);
}

void VulkanRenderer::draw() {
	updateScene();
	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(m_device, 1, &getCurrentFrame().m_renderFence, true, 1000000000));

	getCurrentFrame().m_frameDescriptors.clearPools(m_device);

	uint32_t swapchainImageIndex{};
	VkResult e = vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, getCurrentFrame().m_swapchainSemaphore, nullptr, &swapchainImageIndex);
	if (e == VK_ERROR_OUT_OF_DATE_KHR) {
		m_rendererState->resizeRequested = true;
		return;
	}

	VK_CHECK(vkResetFences(m_device, 1, &getCurrentFrame().m_renderFence));

	// naming it cmd for shorter writing
	auto commandBuffer = getCurrentFrame().m_commandBuffer;

	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

	// begin the command buffer recording. We will use this command buffer exactly once,
	// so we want to let vulkan know that
	auto commandBeginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	m_drawExtent.width = std::min(m_swapchainExtent.width, m_drawImage.imageExtent.width) * m_renderScale;
	m_drawExtent.height = std::min(m_swapchainExtent.height, m_drawImage.imageExtent.height) * m_renderScale;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBeginInfo));

	// transition our main draw image into general layout so we can write into it
	// we will overwrite it all so we dont care about what was the older layout
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	drawBackground(commandBuffer);

	// transition the draw image and the swapchain image into their correct transfer layouts
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	transitionImage(commandBuffer, m_depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	drawGeometry(commandBuffer);

	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	transitionImage(commandBuffer, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	// execute a copy from the draw image into the swapchain
	copyImageToImage(commandBuffer, m_drawImage.image, m_swapchainImages[swapchainImageIndex], m_drawExtent, m_swapchainExtent);

	// set swapchain image layout to Present so we can show it on the screen
	transitionImage(commandBuffer, m_swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// finalize the command buffer (we can no longer add commands, but it can now be executed)
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

	// increase the number of frames drawn
	m_frameNumber++;
}

void VulkanRenderer::drawBackground(VkCommandBuffer commandBuffer) {
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_gradientPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_gradientPipelineLayout, 0, 1, &m_drawImageDescriptors, 0, nullptr);
	vkCmdDispatch(commandBuffer, std::ceil(m_drawExtent.width / 16.0), std::ceil(m_drawExtent.height / 16.0), 1);
}

void VulkanRenderer::drawGeometry(VkCommandBuffer commandBuffer) {
	VkRenderingAttachmentInfo colorAttachment = attachmentInfo(m_drawImage.imageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
	VkRenderingAttachmentInfo depthAttachment = depthAttachmentInfo(m_depthImage.imageView, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = renderingInfo(m_drawExtent, &colorAttachment, &depthAttachment);
	vkCmdBeginRendering(commandBuffer, &renderInfo);

	// Start mesh pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_meshPipeline);

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

	// Bind texture
	VkDescriptorSet imageSet = getCurrentFrame().m_frameDescriptors.allocate(m_device, m_singleImageDescriptorLayout);
	{
		DescriptorWriter writer;
		writer.writeImage(0, errorCheckerboardImage.imageView, defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		writer.updateSet(m_device, imageSet);
	}
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_meshPipelineLayout, 0, 1, &imageSet, 0, nullptr);

	// Calculate view-projection matrix
	// NOTE: Flipping near and far plane to increase depth testing quality
	// NOTE: Invert Y projection since Vulkan has the Y coordinate flipped
	glm::mat4 view = glm::translate(glm::vec3{ 0, 0, -5 });
	glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)m_drawExtent.width / (float)m_drawExtent.height, 10000.f, 0.1f);
	projection[1][1] *= -1;

	// allocate a new uniform buffer for the scene data
	// This should be deleted each frame.
	AllocatedBuffer gpuSceneDataBuffer = createBuffer(sizeof(GPUSceneData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	// write the buffer
	auto* sceneUniformData = static_cast<GPUSceneData*>(gpuSceneDataBuffer.allocation->GetMappedData());
	*sceneUniformData = m_sceneData;

	// create a descriptor set that binds that buffer and update it
	VkDescriptorSet globalDescriptor = getCurrentFrame().m_frameDescriptors.allocate(m_device, m_gpuSceneDataDescriptorLayout);

	DescriptorWriter writer;
	writer.writeBuffer(0, gpuSceneDataBuffer.buffer, sizeof(GPUSceneData), 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.updateSet(m_device, globalDescriptor);

	// Node drawing
	for (const RenderObject& draw : mainDrawContext.OpaqueSurfaces) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 0, 1, &globalDescriptor, 0, nullptr);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline->layout, 1, 1, &draw.material->materialSet, 0, nullptr);

		vkCmdBindIndexBuffer(commandBuffer, draw.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		GPUDrawPushConstants pushConstants{};
		pushConstants.vertexBuffer = draw.vertexBufferAddress;
		pushConstants.worldMatrix = draw.transform;
		vkCmdPushConstants(commandBuffer, draw.material->pipeline->layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);

		vkCmdDrawIndexed(commandBuffer, draw.indexCount, 1, draw.firstIndex, 0, 0);
	}

	// Push Constants
	GPUDrawPushConstants pushConstants{};
	pushConstants.worldMatrix = projection * view;

	// Draw rectangle
	pushConstants.vertexBuffer = rectangle.vertexBufferAddress;
	vkCmdPushConstants(commandBuffer, m_meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
	vkCmdBindIndexBuffer(commandBuffer, rectangle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

	vkCmdEndRendering(commandBuffer);
}

void VulkanRenderer::initDescriptors() {
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	m_globalDescriptorAllocator.init(m_device, 10, sizes);

	// Compute stage image descriptor
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		m_drawImageDescriptorLayout = builder.build(m_device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	// Texture descriptor
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
		m_singleImageDescriptorLayout = builder.build(m_device, VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	// Vertex/Fragment UBO
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
		m_gpuSceneDataDescriptorLayout = builder.build(m_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
	}

	m_drawImageDescriptors = m_globalDescriptorAllocator.allocate(m_device, m_drawImageDescriptorLayout);

	{
		DescriptorWriter writer;
		writer.writeImage(0, m_drawImage.imageView, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		writer.updateSet(m_device, m_drawImageDescriptors);
	}

	for (auto& frame : m_frames) {
		// create a descriptor pool
		std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes = {
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
		};

		frame.m_frameDescriptors = {};
		frame.m_frameDescriptors.init(m_device, 1000, frame_sizes);
	}
}

void VulkanRenderer::initPipelines() {
	initBackgroundPipelines();
	initMeshPipeline();
	metalRoughMaterial.buildPipelines(this);
}

void VulkanRenderer::initBackgroundPipelines() {
	VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &m_drawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VK_CHECK(vkCreatePipelineLayout(m_device, &computeLayout, nullptr, &m_gradientPipelineLayout));

	VkShaderModule computeDrawShader{};
	if (!loadShaderModule("res/shaders/gradient.comp.spv", m_device, &computeDrawShader)) {
		std::cout << std::format("Error when building the compute shader \n");
	}

	auto stageinfo = pipelineShaderStageCreateInfo(VK_SHADER_STAGE_COMPUTE_BIT, computeDrawShader);

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = m_gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_gradientPipeline));

	vkDestroyShaderModule(m_device, computeDrawShader, nullptr);
}

void VulkanRenderer::initMeshPipeline() {

	VkShaderModule triangleFragShader{};
	if (!loadShaderModule("res/shaders/tex_image.frag.spv", m_device, &triangleFragShader)) {
		std::cout << std::format("Error when building the mesh fragment shader module\n");
	}

	VkShaderModule triangleVertexShader{};
	if (!loadShaderModule("res/shaders/mesh.vert.spv", m_device, &triangleVertexShader)) {
		std::cout << std::format("Error when building the mesh vertex shader module\n");
	}

	VkPushConstantRange bufferRange{};
	bufferRange.offset = 0;
	bufferRange.size = sizeof(GPUDrawPushConstants);
	bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = pipelineLayoutCreateInfo();
	pipelineLayoutInfo.pPushConstantRanges = &bufferRange;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_singleImageDescriptorLayout;
	pipelineLayoutInfo.setLayoutCount = 1;

	VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_meshPipelineLayout));

	PipelineBuilder pipelineBuilder;

	pipelineBuilder.setPipelineLayout(m_meshPipelineLayout);
	// connecting the vertex and pixel shaders to the pipeline
	pipelineBuilder.setShaders(triangleVertexShader, triangleFragShader);
	// it will draw triangles
	pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	// filled triangles
	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
	// no backface culling
	pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	// no multisampling
	pipelineBuilder.setMultisamplingNone();
	// no blending
	pipelineBuilder.disableBlending();

	pipelineBuilder.enableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

	pipelineBuilder.setColorAttachmentFormat(m_drawImage.imageFormat);
	pipelineBuilder.setDepthFormat(m_depthImage.imageFormat);

	m_meshPipeline = pipelineBuilder.buildPipeline(m_device);

	vkDestroyShaderModule(m_device, triangleFragShader, nullptr);
	vkDestroyShaderModule(m_device, triangleVertexShader, nullptr);
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

AllocatedBuffer VulkanRenderer::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
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

	return newBuffer;
}

void VulkanRenderer::destroyBuffer(const AllocatedBuffer& buffer) {
	vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation);
}

/*
 * Create a staging buffer in CPU memory to hold the vertex + index buffer data.
 * Copy it to the GPU buffer.
 */
GPUMeshBuffers VulkanRenderer::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface{};

	// create vertex buffer
	newSurface.vertexBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	// find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		.buffer = newSurface.vertexBuffer.buffer
	};
	newSurface.vertexBufferAddress = vkGetBufferDeviceAddress(m_device, &deviceAdressInfo);

	// create index buffer
	newSurface.indexBuffer = createBuffer(indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	AllocatedBuffer staging = createBuffer(vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

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


AllocatedImage VulkanRenderer::createImage(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
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

AllocatedImage VulkanRenderer::createImage(void* data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped) {
	size_t data_size = size.depth * size.width * size.height * 4;
	AllocatedBuffer uploadbuffer = createBuffer(data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

	memcpy(uploadbuffer.info.pMappedData, data, data_size);

	AllocatedImage newImage = createImage(size, format, usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, mipmapped);

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

		transitionImage(cmd, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	});

	destroyBuffer(uploadbuffer);

	return newImage;
}

void VulkanRenderer::destroyImage(const AllocatedImage& img) {
	vkDestroyImageView(m_device, img.imageView, nullptr);
	vmaDestroyImage(m_allocator, img.image, img.allocation);
}

void GLTFMetallic_Roughness::buildPipelines(VulkanRenderer* renderer) {
	VkShaderModule meshFragShader{};
	if (!loadShaderModule("res/shaders/mesh.frag.spv", renderer->m_device, &meshFragShader)) {
		std::cout << std::format("Error when building the triangle fragment shader module") << '\n';
	}

	VkShaderModule meshVertexShader{};
	if (!loadShaderModule("res/shaders/mesh.vert.spv", renderer->m_device, &meshVertexShader)) {
		std::cout << std::format("Error when building the triangle vertex shader module") << '\n';
	}

	VkPushConstantRange matrixRange{};
	matrixRange.offset = 0;
	matrixRange.size = sizeof(GPUDrawPushConstants);
	matrixRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	layoutBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	materialLayout = layoutBuilder.build(renderer->m_device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

	VkDescriptorSetLayout layouts[] = {
		renderer->m_gpuSceneDataDescriptorLayout,
		materialLayout
	};

	VkPipelineLayoutCreateInfo meshLayoutInfo = pipelineLayoutCreateInfo();
	meshLayoutInfo.pPushConstantRanges = &matrixRange;
	meshLayoutInfo.pushConstantRangeCount = 1;
	meshLayoutInfo.setLayoutCount = 2;
	meshLayoutInfo.pSetLayouts = layouts;

	VkPipelineLayout newLayout{};
	VK_CHECK(vkCreatePipelineLayout(renderer->m_device, &meshLayoutInfo, nullptr, &newLayout));

	opaquePipeline.layout = newLayout;
	transparentPipeline.layout = newLayout;

	// build the stage-create-info for both vertex and fragment stages. This lets
	// the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;
	pipelineBuilder.setPipelineLayout(newLayout);
	pipelineBuilder.setShaders(meshVertexShader, meshFragShader);
	pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
	pipelineBuilder.setCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	pipelineBuilder.setMultisamplingNone();
	pipelineBuilder.disableBlending();
	pipelineBuilder.enableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);

	// render format
	pipelineBuilder.setColorAttachmentFormat(renderer->m_drawImage.imageFormat);
	pipelineBuilder.setDepthFormat(renderer->m_depthImage.imageFormat);

	// finally build the pipeline
	opaquePipeline.pipeline = pipelineBuilder.buildPipeline(renderer->m_device);

	// create the transparent variant
	pipelineBuilder.enableBlendingAdditive();

	pipelineBuilder.enableDepthTest(false, VK_COMPARE_OP_GREATER_OR_EQUAL);

	transparentPipeline.pipeline = pipelineBuilder.buildPipeline(renderer->m_device);

	vkDestroyShaderModule(renderer->m_device, meshFragShader, nullptr);
	vkDestroyShaderModule(renderer->m_device, meshVertexShader, nullptr);
}

MaterialInstance GLTFMetallic_Roughness::writeMaterial(VkDevice device, MaterialPass pass, const MaterialResources& resources, DescriptorAllocator& descriptorAllocator) {
	MaterialInstance matData{};
	matData.passType = pass;
	if (pass == MaterialPass::Transparent) {
		matData.pipeline = &transparentPipeline;
	} else {
		matData.pipeline = &opaquePipeline;
	}

	matData.materialSet = descriptorAllocator.allocate(device, materialLayout);


	writer.clear();
	writer.writeBuffer(0, resources.dataBuffer, sizeof(MaterialConstants), resources.dataBufferOffset, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.writeImage(1, resources.colorImage.imageView, resources.colorSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	writer.writeImage(2, resources.metalRoughImage.imageView, resources.metalRoughSampler, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	writer.updateSet(device, matData.materialSet);

	return matData;
}

void MeshNode::draw(const glm::mat4& topMatrix, DrawContext& ctx) {
	glm::mat4 nodeMatrix = topMatrix * worldTransform;

	for (auto& s : mesh->surfaces) {
		RenderObject def{};
		def.indexCount = s.count;
		def.firstIndex = s.startIndex;
		def.indexBuffer = mesh->meshBuffers.indexBuffer.buffer;
		def.material = &s.material->data;

		def.transform = nodeMatrix;
		def.vertexBufferAddress = mesh->meshBuffers.vertexBufferAddress;

		ctx.OpaqueSurfaces.push_back(def);
	}

	// recurse down
	Node::draw(topMatrix, ctx);
}

void VulkanRenderer::updateScene() {
	m_rendererState->mainCamera->update();

	mainDrawContext.OpaqueSurfaces.clear();

	m_sceneData.view = m_rendererState->mainCamera->getViewMatrix();
	// camera projection
	m_sceneData.proj = glm::perspective(glm::radians(70.f), (float)m_rendererState->windowExtent.width / (float)m_rendererState->windowExtent.height, 10000.f, 0.1f);

	// invert the Y direction on projection matrix so that we are more similar
	// to opengl and gltf axis
	m_sceneData.proj[1][1] *= -1;
	m_sceneData.viewproj = m_sceneData.proj * m_sceneData.view;

	// some default lighting parameters
	m_sceneData.ambientColor = glm::vec4(.1f);
	m_sceneData.sunlightColor = glm::vec4(1.f);
	m_sceneData.sunlightDirection = glm::vec4(0, 1, 0.5, 1.f);

	loadedNodes["Suzanne"]->draw(glm::mat4{ 1.f }, mainDrawContext);

	for (int x = -3; x < 3; x++) {

		glm::mat4 scale = glm::scale(glm::vec3{ 0.2 });
		glm::mat4 translation = glm::translate(glm::vec3{ x, 1, 0 });

		loadedNodes["Cube"]->draw(translation * scale, mainDrawContext);
	}

	loadedScenes["structure"]->draw(glm::mat4{ 1.f }, mainDrawContext);
}

}// namespace pm
