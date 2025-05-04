#include "window.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_vulkan.h"

#include "vulkan/buffers.h"

namespace pm {

PrimalWindow createPrimalWindow(std::string_view title, int32_t width, int32_t height, SDL_WindowFlags flags) {

	PrimalWindow window{
		.width = width,
		.height = height,
		.windowFlags = flags,
		.mouseFocus = false,
		.keyboardFocus = false,
		.shown = false,
		.isMinimized = false
	};

	window.handle = SDL_CreateWindow(title.data(), width, height, flags);

	window.id = SDL_GetWindowID(window.handle);

	return window;
}


void destroyPrimalWindow(PrimalWindow* window, VmaAllocator& allocator, VkDevice device, VkInstance instance, VkAllocationCallbacks* callbacks) {
	// TODO(piero): Maybe we shouldn't destroy render targets here?
	if (window->renderTarget.imageView) {
		vkDestroyImageView(device, window->renderTarget.imageView, nullptr);
		vmaDestroyImage(allocator, window->renderTarget.image, window->renderTarget.allocation);
	}

	if (window->depthTarget.imageView) {
		vkDestroyImageView(device, window->depthTarget.imageView, nullptr);
		vmaDestroyImage(allocator, window->depthTarget.image, window->depthTarget.allocation);
	}

	// TODO(piero): Don't destroy buffers here?
	destroyBuffer(allocator, window->uiData);
	destroyBuffer(allocator, window->fontData);

	destroySwapchain(device, &window->swapchain);
	destroyVulkanSurface(instance, window->surface, callbacks);

	SDL_DestroyWindow(window->handle);
}

void getWindowSize(PrimalWindow* window, int* width, int* height) {
	SDL_GetWindowSizeInPixels(window->handle, width, height);
}

void getWindowSizeInPixels(PrimalWindow* window, int* width, int* height) {
	SDL_GetWindowSize(window->handle, width, height);
}

void setWindowRelativeMouseMode(PrimalWindow* window, bool enabled) {
	SDL_SetWindowRelativeMouseMode(window->handle, enabled);
}

VkSurfaceKHR createVulkanSurface(PrimalWindow* window, VkInstance instance, VkAllocationCallbacks* callbacks) {
	VkSurfaceKHR surface{};
	SDL_Vulkan_CreateSurface(window->handle, instance, callbacks, &surface);

	return surface;
}

void destroyVulkanSurface(VkInstance instance, VkSurfaceKHR surface, VkAllocationCallbacks* callbacks) {
	SDL_Vulkan_DestroySurface(instance, surface, callbacks);
}

}// namespace pm
