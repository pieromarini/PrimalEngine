#include "swapchain.h"
#include <VkBootstrap.h>

namespace pm {

PrimalSwapchain createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkFormat format, VkPresentModeKHR presentMode) {
	PrimalSwapchain p{};

	vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
																	.set_desired_format(VkSurfaceFormatKHR{ .format = format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
																	.set_desired_present_mode(presentMode)
																	.set_desired_extent(width, height)
																	.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
																	.build()
																	.value();

	p.extent = vkbSwapchain.extent;
	p.handle = vkbSwapchain.swapchain;
	p.images = vkbSwapchain.get_images().value();
	p.imageViews = vkbSwapchain.get_image_views().value();

	return p;
}

void destroySwapchain(VkDevice device, PrimalSwapchain* swapchain) {
	vkDestroySwapchainKHR(device, swapchain->handle, nullptr);

	for (auto& swapchainImageView : swapchain->imageViews) {
		vkDestroyImageView(device, swapchainImageView, nullptr);
	}
}

}// namespace pm
