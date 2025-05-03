#pragma once

#include <vector>
#include <array>
#include <vulkan/vulkan.h>

namespace pm {

struct PrimalSwapchain {
	VkSwapchainKHR handle;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;

	std::array<VkSemaphore, 2> swapchainSemaphores{};
	std::array<VkSemaphore, 2> renderSemaphores{};
};


PrimalSwapchain createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkFormat format, VkPresentModeKHR presentMode);
void destroySwapchain(VkDevice device, PrimalSwapchain* swapchain);

};// namespace pm
