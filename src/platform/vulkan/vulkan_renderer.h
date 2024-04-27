#pragma once

#include "vk_types.h"
#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

namespace pm {

struct VulkanRendererConfig {
		bool useValidationLayers;
		VkExtent2D windowExtent;
		SDL_Window* window;
};

struct FrameData {
		VkCommandPool m_commandPool;
		VkCommandBuffer m_commandBuffer;
		VkSemaphore m_swapchainSemaphore;
		VkSemaphore m_renderSemaphore;
		VkFence m_renderFence;
};

struct AllocatedImage {
		VkImage image;
		VkImageView imageView;
		VmaAllocation allocation;
		VkExtent3D imageExtent;
		VkFormat imageFormat;
};

constexpr uint32_t FRAME_OVERLAP = 2;


class VulkanRenderer {
	public:
		void init(VulkanRendererConfig config);
		void initVulkan(VulkanRendererConfig& config);
		void initSwapchain(VulkanRendererConfig& config);
		void initCommands(VulkanRendererConfig& config);
		void initSyncStructures(VulkanRendererConfig& config);

		// drawing
		void draw();
		void drawBackground(VkCommandBuffer commandBuffer);

		void cleanup();

	private:
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debug_messenger;
		VkPhysicalDevice m_chosenGPU;
		VkDevice m_device;
		VkSurfaceKHR m_surface;

		// Swapchain
		void createSwapchain(uint32_t width, uint32_t height);
		void destroySwapchain();
		VkSwapchainKHR m_swapchain;
		VkFormat m_swapchainImageFormat;
		std::vector<VkImage> m_swapchainImages;
		std::vector<VkImageView> m_swapchainImageViews;
		VkExtent2D m_swapchainExtent;

		// Commands
		FrameData m_frames[FRAME_OVERLAP]{};
		uint32_t m_frameNumber{};
		FrameData& getCurrentFrame() { return m_frames[m_frameNumber % FRAME_OVERLAP]; };
		VkQueue m_graphicsQueue{};
		uint32_t m_graphicsQueueFamily{};

		// Allocator
		VmaAllocator m_allocator;

		// Draw resources
		AllocatedImage m_drawImage;
		VkExtent2D m_drawExtent;
};

}// namespace pm
