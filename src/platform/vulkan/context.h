#pragma once

#include <set>
#include <vulkan/vulkan.hpp>
#include "../../primal_window.h"
#include "../../utils/fileLoader.h"

#include "validation.h"
#include "queues.h"
#include "swapchain.h"
#include "shader_module.h"

namespace primal {

constexpr int MAX_FRAMES_IN_FLIGHT = 2;

class VulkanContext {

  public:
	VulkanContext(Window* window) : m_window{ window } {}

	~VulkanContext() {
		cleanupSwapChain();

		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			m_device.destroySemaphore(m_renderFinishedSemaphores[i]);
			m_device.destroySemaphore(m_imageAvailableSemaphores[i]);
			m_device.destroyFence(m_inFlightFences[i]);
		}

		m_device.destroyCommandPool(m_commandPool);

		m_device.destroy();

		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}

		m_instance.destroySurfaceKHR(m_surface);
		m_instance.destroy();
	}

	void cleanupSwapChain() {
		for (auto framebuffer : m_swapChainFramebuffers) {
			m_device.destroyFramebuffer(framebuffer);
		}

		m_device.destroyPipeline(m_graphicsPipeline);
		m_device.destroyPipelineLayout(m_pipelineLayout);
		m_device.destroyRenderPass(m_renderPass);

		for (auto imageView : m_swapChainImageViews) {
			m_device.destroyImageView(imageView);
		}

		m_device.destroySwapchainKHR(m_swapChain);
	}

	void recreateSwapChain() {
		m_device.waitIdle();

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
	}


	void init() {
		createApplication();
		setupDebugMessenger(m_instance, m_debugMessenger);
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
		createFramebuffers();
		createCommandPool();
		createCommandBuffers();
		createSyncObjects();
	}

	void createApplication() {
		if (enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("[VULKAN] Validation layers requested, but not available.");
		}
		vk::ApplicationInfo appInfo{};
		appInfo.sType = vk::StructureType::eApplicationInfo;
		appInfo.pApplicationName = "Primal Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Primal";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		vk::InstanceCreateInfo createInfo{};
		createInfo.sType = vk::StructureType::eInstanceCreateInfo;
		createInfo.pApplicationInfo = &appInfo;

		// DEBUG: Get supported extensions from GLFW
		auto extensions = m_window->getGLFWExtensions(enableValidationLayers);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();


		vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		} else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vk::createInstance(&createInfo, nullptr, &m_instance) != vk::Result::eSuccess) {
			throw std::runtime_error("[VULKAN] Failed to create instance.");
		}
	}

	void createSurface() {
		auto vkSurface = VkSurfaceKHR(m_surface);

		m_window->createWindowSurface(m_instance, vkSurface);

		m_surface = vk::SurfaceKHR(vkSurface);
	}

	void pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("[VULKAN] No GPU found with Vulkan Support");
		}

		auto devices = m_instance.enumeratePhysicalDevices();

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				m_physicalDevice = device;
				break;
			}
		}

		if (!m_physicalDevice) {
			throw std::runtime_error("[VULKAN] Failed to find a suitable GPU");
		}
	}

	// TODO: Query all extensions/requirements for the engine
	bool isDeviceSuitable(vk::PhysicalDevice device) {
		auto indices = findQueueFamilies(device, m_surface);

		bool extensionsSupported = checkDeviceExtensionSupport(device);
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			auto swapChainSupport = querySwapChainSupport(device, m_surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

	bool checkDeviceExtensionSupport(vk::PhysicalDevice device) {
		auto availableExtensions = device.enumerateDeviceExtensionProperties();

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	void createLogicalDevice() {
		auto indices = findQueueFamilies(m_physicalDevice, m_surface);

		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			vk::DeviceQueueCreateInfo queueCreateInfo{
				{},
				queueFamily,
				1,
				&queuePriority,
			};
			queueCreateInfos.emplace_back(queueCreateInfo);
		}

		vk::PhysicalDeviceFeatures deviceFeatures{};

		vk::DeviceCreateInfo deviceCreateInfo{
			{},
			static_cast<uint32_t>(queueCreateInfos.size()),
			queueCreateInfos.data(),
			static_cast<uint32_t>(validationLayers.size()),
			validationLayers.data(),
			static_cast<uint32_t>(deviceExtensions.size()),
			deviceExtensions.data(),
			&deviceFeatures
		};

		m_device = m_physicalDevice.createDevice(deviceCreateInfo);
		m_graphicsQueue = m_device.getQueue(indices.graphicsFamily.value(), 0);
		m_presentQueue = m_device.getQueue(indices.presentFamily.value(), 0);
	}

	void createSwapChain() {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice, m_surface);

		auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		auto extent = chooseSwapExtent(swapChainSupport.capabilities, m_window);

		uint32_t imageCount{};
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		vk::SwapchainCreateInfoKHR createInfo{
			{},
			m_surface,
			imageCount,
			surfaceFormat.format,
			surfaceFormat.colorSpace,
			extent,
			1,
			vk::ImageUsageFlagBits::eColorAttachment,
			vk::SharingMode::eExclusive,
			0,
			nullptr,
			swapChainSupport.capabilities.currentTransform,
			vk::CompositeAlphaFlagBitsKHR::eOpaque,
			presentMode,
			true,
			nullptr
		};

		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);
		std::array<uint32_t, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}

		m_swapChain = m_device.createSwapchainKHR(createInfo, nullptr);
		m_swapChainImages = m_device.getSwapchainImagesKHR(m_swapChain);

		m_swapChainImageFormat = surfaceFormat.format;
		m_swapChainExtent = extent;
	}

	void createImageViews() {
		m_swapChainImageViews.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImages.size(); i++) {
			vk::ImageViewCreateInfo createInfo{
				{},
				m_swapChainImages[i],
				vk::ImageViewType::e2D,
				m_swapChainImageFormat,
				{ vk::ComponentSwizzle::eIdentity,
				  vk::ComponentSwizzle::eIdentity,
				  vk::ComponentSwizzle::eIdentity,
				  vk::ComponentSwizzle::eIdentity },
				{ vk::ImageAspectFlagBits::eColor,
				  0,
				  1,
				  0,
				  1 }
			};
			m_swapChainImageViews[i] = m_device.createImageView(createInfo);
		}
	}

	void createRenderPass() {
		vk::AttachmentDescription colorAttachment{
			{},
			m_swapChainImageFormat,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		};

		vk::AttachmentReference colorAttachmentRef{
			0,
			vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::SubpassDescription subpass{
			{},
			vk::PipelineBindPoint::eGraphics,
			0,
			nullptr,
			1,
			&colorAttachmentRef,
			nullptr,
			nullptr,
			0,
			nullptr
		};

		vk::SubpassDependency dependency{
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::AccessFlagBits::eNone,
			vk::AccessFlagBits::eColorAttachmentWrite
		};

		vk::RenderPassCreateInfo renderPassInfo{
			{},
			1,
			&colorAttachment,
			1,
			&subpass,
			1,
			&dependency
		};


		m_renderPass = m_device.createRenderPass(renderPassInfo);
	}

	void createGraphicsPipeline() {
		auto vertShaderCode = readFile("res/shaders/default.vert.spv");
		auto fragShaderCode = readFile("res/shaders/default.frag.spv");

		auto vertShaderModule = createShaderModule(m_device, vertShaderCode);
		auto fragShaderModule = createShaderModule(m_device, fragShaderCode);

		// Shaders
		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
			{},
			vk::ShaderStageFlagBits::eVertex,
			vertShaderModule,
			"main",
			nullptr
		};
		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
			{},
			vk::ShaderStageFlagBits::eFragment,
			fragShaderModule,
			"main",
			nullptr
		};
		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		// Vertex
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
			{},
			0,
			nullptr,
			0,
			nullptr
		};

		// Assembly/Topology
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
			{},
			vk::PrimitiveTopology::eTriangleList,
			false
		};

		// Viewport/Scissor
		vk::Viewport viewport{
			0.0f, 0.0f, static_cast<float>(m_swapChainExtent.width), static_cast<float>(m_swapChainExtent.height), 0.0f, 1.0f
		};

		vk::Rect2D scissor{
			{ 0, 0 },
			m_swapChainExtent
		};

		vk::PipelineViewportStateCreateInfo viewportState{
			{},
			1,
			&viewport,
			1,
			&scissor
		};

		// Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer{
			{},
			false,
			false,
			vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack,
			vk::FrontFace::eClockwise,
			false,
			0.0f,
			0.0f,
			0.0f,
			1.0f
		};

		// Multisampling
		vk::PipelineMultisampleStateCreateInfo multisampling{
			{},
			vk::SampleCountFlagBits::e1,
			false,
			1.0f,
			nullptr,
			false,
			false
		};

		// Color Blending
		vk::PipelineColorBlendAttachmentState colorBlendAttachment{
			false,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::BlendFactor::eOne,
			vk::BlendFactor::eZero,
			vk::BlendOp::eAdd,
			vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending{
			{},
			false,
			vk::LogicOp::eCopy,
			1,
			&colorBlendAttachment,
			{ 0.0f, 0.0f, 0.0f, 0.0f }
		};

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
			{},
			0,
			nullptr,
			0,
			nullptr
		};
		m_pipelineLayout = m_device.createPipelineLayout(pipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo pipelineInfo{
			{},
			2,
			shaderStages.data(),
			&vertexInputInfo,
			&inputAssembly,
			nullptr,
			&viewportState,
			&rasterizer,
			&multisampling,
			nullptr,
			&colorBlending,
			nullptr,
			m_pipelineLayout,
			m_renderPass,
			0,
			nullptr,
			-1
		};

		vk::Result result{};
		std::tie(result, m_graphicsPipeline) = m_device.createGraphicsPipeline(nullptr, pipelineInfo);

		m_device.destroyShaderModule(vertShaderModule);
		m_device.destroyShaderModule(fragShaderModule);
	}

	void createFramebuffers() {
		m_swapChainFramebuffers.resize(m_swapChainImageViews.size());
		for (size_t i = 0; i < m_swapChainImageViews.size(); i++) {
			vk::ImageView attachments[] = {
				m_swapChainImageViews[i]
			};

			vk::FramebufferCreateInfo framebufferInfo{
				{},
				m_renderPass,
				1,
				attachments,
				m_swapChainExtent.width,
				m_swapChainExtent.height,
				1
			};
			[[maybe_unused]] auto result = m_device.createFramebuffer(&framebufferInfo, nullptr, &m_swapChainFramebuffers[i]);
		}
	}

	void createCommandPool() {
		auto queueFamilyIndices = findQueueFamilies(m_physicalDevice, m_surface);

		vk::CommandPoolCreateInfo poolInfo{
			vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			queueFamilyIndices.graphicsFamily.value()
		};

		m_commandPool = m_device.createCommandPool(poolInfo);
	}

	void createCommandBuffers() {
		m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		vk::CommandBufferAllocateInfo allocInfo{
			m_commandPool,
			vk::CommandBufferLevel::ePrimary,
			static_cast<uint32_t>(m_commandBuffers.size())
		};

		m_commandBuffers = m_device.allocateCommandBuffers(allocInfo);
	}

	void recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex) {
		vk::CommandBufferBeginInfo beginInfo{
			{},
			nullptr
		};
		[[maybe_unused]] auto result = commandBuffer.begin(&beginInfo);

		vk::ClearValue clearColor{ std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };

		vk::RenderPassBeginInfo renderPassInfo{
			m_renderPass,
			m_swapChainFramebuffers[imageIndex],
			{ { 0, 0 }, m_swapChainExtent },
			1,
			&clearColor
		};

		commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphicsPipeline);
		commandBuffer.draw(3, 1, 0, 0);

		commandBuffer.endRenderPass();

		commandBuffer.end();
	}

	void createSyncObjects() {
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		vk::SemaphoreCreateInfo semaphoreInfo{};
		vk::FenceCreateInfo fenceInfo{
			vk::FenceCreateFlagBits::eSignaled
		};

		for (std::size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
			m_imageAvailableSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
			m_renderFinishedSemaphores[i] = m_device.createSemaphore(semaphoreInfo);
			m_inFlightFences[i] = m_device.createFence(fenceInfo);
		}
	}

	/*
		1) Wait for the previous frame to finish
		2) Acquire an image from the swap chain
		3) Record a command buffer which draws the scene onto that image
		4) Submit the recorded command buffer
		5) Present the swap chain image
	*/
	void drawFrame() {
		vk::Result result{};
		result = m_device.waitForFences(m_inFlightFences[currentFrame], true, UINT64_MAX);

		uint32_t imageIndex{};
		result = m_device.acquireNextImageKHR(m_swapChain, UINT64_MAX, m_imageAvailableSemaphores[currentFrame], nullptr, &imageIndex);
		if (result == vk::Result::eErrorOutOfDateKHR) {
			recreateSwapChain();
			return;
		} else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) {
			throw std::runtime_error("[VULKAN] Failed to acquire swap chain image");
		}

		m_device.resetFences(m_inFlightFences[currentFrame]);

		m_commandBuffers[currentFrame].reset();
		recordCommandBuffer(m_commandBuffers[currentFrame], imageIndex);

		std::array<vk::Semaphore, 1> waitSemaphores = { m_imageAvailableSemaphores[currentFrame] };
		std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		std::array<vk::Semaphore, 1> signalSemaphores = { m_renderFinishedSemaphores[currentFrame] };
		vk::SubmitInfo submitInfo{
			1,
			waitSemaphores.data(),
			waitStages.data(),
			1,
			&m_commandBuffers[currentFrame],
			1,
			signalSemaphores.data()
		};
		m_graphicsQueue.submit(submitInfo, m_inFlightFences[currentFrame]);

		std::array<vk::SwapchainKHR, 1> swapChains = { m_swapChain };
		vk::PresentInfoKHR presentInfo{
			1,
			signalSemaphores.data(),
			1,
			swapChains.data(),
			&imageIndex
		};

		result = m_presentQueue.presentKHR(presentInfo);
		if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || m_framebufferResized) {
			m_framebufferResized = false;
			recreateSwapChain();
		} else if (result != vk::Result::eSuccess) {
			throw std::runtime_error("[VULKAN] Failed to present swap chain image");
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void waitIdle() {
		m_device.waitIdle();
	}

	void setFramebufferResized(bool flag) {
		m_framebufferResized = flag;
	}

  private:
	Window* m_window;
	vk::Instance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;
	vk::PhysicalDevice m_physicalDevice;
	vk::Device m_device;
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;
	vk::SurfaceKHR m_surface;
	vk::SwapchainKHR m_swapChain;
	std::vector<vk::Image> m_swapChainImages;
	std::vector<vk::ImageView> m_swapChainImageViews;
	vk::Format m_swapChainImageFormat;
	vk::RenderPass m_renderPass;
	vk::PipelineLayout m_pipelineLayout;
	vk::Extent2D m_swapChainExtent;
	vk::Pipeline m_graphicsPipeline;
	std::vector<vk::Framebuffer> m_swapChainFramebuffers;
	vk::CommandPool m_commandPool;
	std::vector<vk::CommandBuffer> m_commandBuffers;
	std::vector<vk::Semaphore> m_imageAvailableSemaphores;
	std::vector<vk::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::Fence> m_inFlightFences;

	uint32_t currentFrame = 0;
	bool m_framebufferResized = false;


	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
};

}// namespace primal
