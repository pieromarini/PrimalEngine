#include "swapchain.h"
#include "platform/vulkan/vulkan_structures_helpers.h"
#include <VkBootstrap.h>

namespace pm {

PrimalSwapchain createSwapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, uint32_t width, uint32_t height, VkFormat format, VkPresentModeKHR presentMode) {
	PrimalSwapchain newSwapchain{};

	vkb::SwapchainBuilder swapchainBuilder{ physicalDevice, device, surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
																	.set_desired_format(VkSurfaceFormatKHR{ .format = format, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
																	.set_desired_present_mode(presentMode)
																	.set_desired_extent(width, height)
																	.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
																	.build()
																	.value();

	newSwapchain.extent = vkbSwapchain.extent;
	newSwapchain.handle = vkbSwapchain.swapchain;
	newSwapchain.images = vkbSwapchain.get_images().value();
	newSwapchain.imageViews = vkbSwapchain.get_image_views().value();

	auto semaphoreCreate = semaphoreCreateInfo();
	for (auto& semaphore: newSwapchain.swapchainSemaphores) {
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreate, nullptr, &semaphore));
	}
	for (auto& semaphore: newSwapchain.renderSemaphores) {
		VK_CHECK(vkCreateSemaphore(device, &semaphoreCreate, nullptr, &semaphore));
	}

	return newSwapchain;
}

void destroySwapchain(VkDevice device, PrimalSwapchain* swapchain) {
	for (auto semaphore: swapchain->swapchainSemaphores) {
		if (semaphore) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	}
	for (auto semaphore: swapchain->renderSemaphores) {
		if (semaphore) {
			vkDestroySemaphore(device, semaphore, nullptr);
		}
	}

	vkDestroySwapchainKHR(device, swapchain->handle, nullptr);

	for (auto& swapchainImageView : swapchain->imageViews) {
		vkDestroyImageView(device, swapchainImageView, nullptr);
	}
}

}// namespace pm
