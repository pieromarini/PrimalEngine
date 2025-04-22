#include "window.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_vulkan.h"

namespace pm {

PrimalWindow createPrimalWindow(std::string_view title, int32_t width, int32_t height, SDL_WindowFlags flags) {
	static uint32_t idCounter{ 0 };

	PrimalWindow window{
		.id = idCounter++,
		.windowFlags = flags,
		.width = width,
		.height = height,
		.hasFocus = false
	};

	window.handle = SDL_CreateWindow(title.data(), width, height, flags);

	return window;
}


void destroyPrimalWindow(PrimalWindow* window) {
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

}// namespace pm
