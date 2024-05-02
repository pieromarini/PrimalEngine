#pragma once

#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>
#include "vulkan_loader.h"

#include "vk_types.h"
#include "vulkan_descriptor.h"

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

		// NOTE: load some default data for our engine to draw
		void initDefaultData();

		// drawing
		void draw();
		void drawBackground(VkCommandBuffer commandBuffer);
		void drawGeometry(VkCommandBuffer commandBuffer);

		void cleanup();

		// Buffers
		AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		void destroyBuffer(const AllocatedBuffer& buffer);
		GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

		void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	private:
		void initVulkan(VulkanRendererConfig& config);
		void initSwapchain(VulkanRendererConfig& config);
		void initCommands(VulkanRendererConfig& config);
		void initSyncStructures(VulkanRendererConfig& config);
		void initDescriptors();
		void initPipelines();

		// specific pipelines
		void initBackgroundPipelines();
		void initMeshPipeline();

		// Structures for immediateSubmit
    VkFence m_immFence;
    VkCommandBuffer m_immCommandBuffer;
    VkCommandPool m_immCommandPool;

		// Vulkan init stuff
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
		AllocatedImage m_depthImage;
		VkExtent2D m_drawExtent;

		// Descriptors
		DescriptorAllocator m_globalDescriptorAllocator;
		VkDescriptorSet m_drawImageDescriptors;
		VkDescriptorSetLayout m_drawImageDescriptorLayout;

		// Compute pipeline
		VkPipeline m_gradientPipeline;
		VkPipelineLayout m_gradientPipelineLayout;

		// Mesh pipeline
		VkPipeline m_meshPipeline;
		VkPipelineLayout m_meshPipelineLayout;

		// Loaded meshes from GLTF file
		std::vector<std::shared_ptr<MeshAsset>> m_testMeshes;

};

}// namespace pm
