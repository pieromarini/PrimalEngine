#define VMA_IMPLEMENTATION
#include "vulkan_renderer.h"
#include "SDL3/SDL_vulkan.h"
#include "platform/vulkan/vulkan_descriptor.h"
#include "platform/vulkan/vulkan_images.h"
#include "platform/vulkan/vulkan_loader.h"
#include "vk_types.h"
#include "vulkan_pipeline.h"
#include "vulkan_shader.h"
#include "vulkan_structures_helpers.h"
#include <cmath>
#include <vector>

namespace pm {

void VulkanRenderer::init(VulkanRendererConfig config) {
	initVulkan(config);
	initSwapchain(config);
	initCommands(config);
	initSyncStructures(config);
	initDescriptors();
	initPipelines();
	initDefaultData();
}

// Hardcoding an indexed rectangle
void VulkanRenderer::initDefaultData() {
	std::array<Vertex, 4> rectVertices{};

	rectVertices[0].position = { 0.5, -0.5, 0 };
	rectVertices[1].position = { 0.5, 0.5, 0 };
	rectVertices[2].position = { -0.5, -0.5, 0 };
	rectVertices[3].position = { -0.5, 0.5, 0 };

	rectVertices[0].color = { 0, 0, 0, 1 };
	rectVertices[1].color = { 0.5, 0.5, 0.5, 1 };
	rectVertices[2].color = { 1, 0, 0, 1 };
	rectVertices[3].color = { 0, 1, 0, 1 };

	std::array<uint32_t, 6> rectIndices{};

	rectIndices[0] = 0;
	rectIndices[1] = 1;
	rectIndices[2] = 2;

	rectIndices[3] = 2;
	rectIndices[4] = 1;
	rectIndices[5] = 3;

	rectangle = uploadMesh(rectIndices, rectVertices);

	m_testMeshes = loadGltfMeshes(this, "res/models/basicmesh.glb").value();
}

void VulkanRenderer::initVulkan(VulkanRendererConfig& config) {
	vkb::InstanceBuilder builder;

	// make the vulkan instance, with basic debug features
	auto inst = builder.set_app_name("Primal Engine")
								.request_validation_layers(config.useValidationLayers)
								.use_default_debug_messenger()
								.require_api_version(1, 3, 0)
								.build();

	auto vkbInstance = inst.value();

	// grab the instance
	m_instance = vkbInstance.instance;
	m_debug_messenger = vkbInstance.debug_messenger;

	SDL_Vulkan_CreateSurface(config.window, m_instance, nullptr, &m_surface);

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

	std::cout << std::format("Selected PhysicalDevice: {}\n", physicalDevice.name);


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

void VulkanRenderer::initSwapchain(VulkanRendererConfig& config) {
	createSwapchain(config.windowExtent.width, config.windowExtent.height);

	// draw image size will match the window
	VkExtent3D drawImageExtent = {
		config.windowExtent.width,
		config.windowExtent.height,
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

	// for the draw image, we want to allocate it from gpu local memory
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	vmaCreateImage(m_allocator, &rimg_info, &rimg_allocinfo, &m_drawImage.image, &m_drawImage.allocation, nullptr);

	// build a image-view for the draw image to use for rendering
	VkImageViewCreateInfo rview_info = imageViewCreateInfo(m_drawImage.imageFormat, m_drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

	VK_CHECK(vkCreateImageView(m_device, &rview_info, nullptr, &m_drawImage.imageView));
}

void VulkanRenderer::initCommands(VulkanRendererConfig& config) {
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

void VulkanRenderer::initSyncStructures(VulkanRendererConfig& config) {
	// create syncronization structures
	// one fence to control when the gpu has finished rendering the frame,
	// and 2 semaphores to syncronize rendering with swapchain
	// we want the fence to start signalled so we can wait on it on the first frame
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

	for (auto& frame : m_frames) {
		vkDestroyCommandPool(m_device, frame.m_commandPool, nullptr);

		vkDestroyFence(m_device, frame.m_renderFence, nullptr);
		vkDestroySemaphore(m_device, frame.m_renderSemaphore, nullptr);
		vkDestroySemaphore(m_device, frame.m_swapchainSemaphore, nullptr);
	}

	vkDestroyCommandPool(m_device, m_immCommandPool, nullptr);
	vkDestroyFence(m_device, m_immFence, nullptr);

	vkDestroyImageView(m_device, m_drawImage.imageView, nullptr);
	vmaDestroyImage(m_allocator, m_drawImage.image, m_drawImage.allocation);

	vkDestroyPipelineLayout(m_device, m_gradientPipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_gradientPipeline, nullptr);

	vkDestroyPipelineLayout(m_device, m_trianglePipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_trianglePipeline, nullptr);

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
	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(m_device, 1, &getCurrentFrame().m_renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(m_device, 1, &getCurrentFrame().m_renderFence));

	uint32_t swapchainImageIndex{};
	VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain, 1000000000, getCurrentFrame().m_swapchainSemaphore, nullptr, &swapchainImageIndex));

	// naming it cmd for shorter writing
	auto commandBuffer = getCurrentFrame().m_commandBuffer;

	// now that we are sure that the commands finished executing, we can safely
	// reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(commandBuffer, 0));

	// begin the command buffer recording. We will use this command buffer exactly once,
	// so we want to let vulkan know that
	auto commandBeginInfo = commandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	m_drawExtent.width = m_drawImage.imageExtent.width;
	m_drawExtent.height = m_drawImage.imageExtent.height;

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &commandBeginInfo));

	// transition our main draw image into general layout so we can write into it
	// we will overwrite it all so we dont care about what was the older layout
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	drawBackground(commandBuffer);

	// transition the draw image and the swapchain image into their correct transfer layouts
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

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

	VK_CHECK(vkQueuePresentKHR(m_graphicsQueue, &presentInfo));

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

	VkRenderingInfo renderInfo = renderingInfo(m_drawExtent, &colorAttachment, nullptr);
	vkCmdBeginRendering(commandBuffer, &renderInfo);

	// Start triangle pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_trianglePipeline);

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

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	// Start mesh pipeline
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_meshPipeline);

	// Upload constants
	GPUDrawPushConstants pushConstants{};
	pushConstants.worldMatrix = glm::mat4{ 1.f };
	pushConstants.vertexBuffer = rectangle.vertexBufferAddress;

	// Draw rectangle
	vkCmdPushConstants(commandBuffer, m_meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
	vkCmdBindIndexBuffer(commandBuffer, rectangle.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

	// Draw GLTF mesh
	pushConstants.vertexBuffer = m_testMeshes[2]->meshBuffers.vertexBufferAddress;
	vkCmdPushConstants(commandBuffer, m_meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &pushConstants);
	vkCmdBindIndexBuffer(commandBuffer, m_testMeshes[2]->meshBuffers.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
	vkCmdDrawIndexed(commandBuffer, m_testMeshes[2]->surfaces[0].count, 1, m_testMeshes[2]->surfaces[0].startIndex, 0, 0);

	vkCmdEndRendering(commandBuffer);
}

void VulkanRenderer::initDescriptors() {
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	m_globalDescriptorAllocator.initPool(m_device, 10, sizes);

	DescriptorLayoutBuilder builder;
	builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	m_drawImageDescriptorLayout = builder.build(m_device, VK_SHADER_STAGE_COMPUTE_BIT);

	m_drawImageDescriptors = m_globalDescriptorAllocator.allocate(m_device, m_drawImageDescriptorLayout);

	VkDescriptorImageInfo imgInfo = {};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = m_drawImage.imageView;

	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;

	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = m_drawImageDescriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(m_device, 1, &drawImageWrite, 0, nullptr);
}

void VulkanRenderer::initPipelines() {
	initBackgroundPipelines();
	initTrianglePipeline();
	initMeshPipeline();
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

void VulkanRenderer::initTrianglePipeline() {
	VkShaderModule triangleFragShader{};
	if (!loadShaderModule("res/shaders/triangle.frag.spv", m_device, &triangleFragShader)) {
		std::cout << std::format("Error when building the triangle fragment shader module\n");
	} else {
		std::cout << std::format("Triangle fragment shader succesfully loaded\n");
	}

	VkShaderModule triangleVertexShader{};
	if (!loadShaderModule("res/shaders/triangle.vert.spv", m_device, &triangleVertexShader)) {
		std::cout << std::format("Error when building the triangle vertex shader module\n");
	} else {
		std::cout << std::format("Triangle vertex shader succesfully loaded\n");
	}

	// build the pipeline layout that controls the inputs/outputs of the shader
	// we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = pipelineLayoutCreateInfo();
	VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_trianglePipelineLayout));

	PipelineBuilder pipelineBuilder;

	// use the triangle layout we created
	pipelineBuilder.setPipelineLayout(m_trianglePipelineLayout);
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
	// no depth testing
	pipelineBuilder.disableDepthTest();

	// connect the image format we will draw into, from draw image
	pipelineBuilder.setColorAttachmentFormat(m_drawImage.imageFormat);
	pipelineBuilder.setDepthFormat(VK_FORMAT_UNDEFINED);

	// finally build the pipeline
	m_trianglePipeline = pipelineBuilder.buildPipeline(m_device);

	vkDestroyShaderModule(m_device, triangleFragShader, nullptr);
	vkDestroyShaderModule(m_device, triangleVertexShader, nullptr);
}

void VulkanRenderer::initMeshPipeline() {
	VkShaderModule triangleFragShader{};
	if (!loadShaderModule("res/shaders/triangle.frag.spv", m_device, &triangleFragShader)) {
		std::cout << std::format("Error when building the mesh fragment shader module\n");
	} else {
		std::cout << std::format("Triangle mesh shader succesfully loaded\n");
	}

	VkShaderModule triangleVertexShader{};
	if (!loadShaderModule("res/shaders/mesh.vert.spv", m_device, &triangleVertexShader)) {
		std::cout << std::format("Error when building the mesh vertex shader module\n");
	} else {
		std::cout << std::format("Triangle mesh shader succesfully loaded\n");
	}

	VkPushConstantRange bufferRange{};
	bufferRange.offset = 0;
	bufferRange.size = sizeof(GPUDrawPushConstants);
	bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = pipelineLayoutCreateInfo();
	pipelineLayoutInfo.pPushConstantRanges = &bufferRange;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_meshPipelineLayout));

	PipelineBuilder pipelineBuilder;

	// use the triangle layout we created
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

	pipelineBuilder.disableDepthTest();

	// connect the image format we will draw into, from draw image
	pipelineBuilder.setColorAttachmentFormat(m_drawImage.imageFormat);
	pipelineBuilder.setDepthFormat(VK_FORMAT_UNDEFINED);

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

GPUMeshBuffers VulkanRenderer::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
	const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	GPUMeshBuffers newSurface{};

	// create vertex buffer
	newSurface.vertexBuffer = createBuffer(vertexBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

	// find the adress of the vertex buffer
	VkBufferDeviceAddressInfo deviceAdressInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = newSurface.vertexBuffer.buffer };
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
	void initMeshPipeline();

	destroyBuffer(staging);

	return newSurface;
}

}// namespace pm
