#include <SDL3/SDL.h>
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

namespace pm {

struct VulkanRendererConfig {
		bool useValidationLayers;
		VkExtent2D windowExtent;
		SDL_Window* window;
};

class VulkanRenderer {
	public:
		void init(VulkanRendererConfig config);
		void initVulkan(VulkanRendererConfig& config);
		void initSwapchain(VulkanRendererConfig& config);
		void initCommands(VulkanRendererConfig& config);
		void initSyncStructures(VulkanRendererConfig& config);

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
};

}// namespace pm
