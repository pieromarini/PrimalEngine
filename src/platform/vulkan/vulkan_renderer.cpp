#define VMA_IMPLEMENTATION
#include <vector>
#include <cmath>
#include "SDL3/SDL_vulkan.h"
#include "platform/vulkan/vulkan_images.h"
#include "vk_types.h"
#include "vulkan_renderer.h"
#include "vulkan_structures_helpers.h"
#include "platform/vulkan/vulkan_descriptor.h"

namespace pm {

void VulkanRenderer::init(VulkanRendererConfig config) {
	initVulkan(config);
	initSwapchain(config);
	initCommands(config);
	initSyncStructures(config);
	initDescriptors();
	initPipelines();
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

	vmaDestroyAllocator(m_allocator);

	for (auto& frame : m_frames) {
		vkDestroyCommandPool(m_device, frame.m_commandPool, nullptr);

		vkDestroyFence(m_device, frame.m_renderFence, nullptr);
		vkDestroySemaphore(m_device, frame.m_renderSemaphore, nullptr);
		vkDestroySemaphore(m_device, frame.m_swapchainSemaphore, nullptr);
	}

	vkDestroyImageView(m_device, m_drawImage.imageView, nullptr);
	vmaDestroyImage(m_allocator, m_drawImage.image, m_drawImage.allocation);

	vkDestroyPipelineLayout(m_device, m_gradientPipelineLayout, nullptr);
	vkDestroyPipeline(m_device, m_gradientPipeline, nullptr);

	destroySwapchain();

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
	transitionImage(commandBuffer, m_drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
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
		std::format("Error when building the compute shader \n");
	}

	VkPipelineShaderStageCreateInfo stageinfo{};
	stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageinfo.pNext = nullptr;
	stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageinfo.module = computeDrawShader;
	stageinfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = m_gradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;

	VK_CHECK(vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_gradientPipeline));

	// Shader modules are only used when building the pipeline
	vkDestroyShaderModule(m_device, computeDrawShader, nullptr);
}


}// namespace pm
