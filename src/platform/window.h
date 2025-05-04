#pragma once

#include "SDL3/SDL_video.h"
#include "platform/vulkan/swapchain.h"
#include "vk_types.h"
#include <string_view>
#include <vulkan/vulkan_core.h>

namespace pm {

struct PrimalWindow {
	uint32_t id{};
	int32_t width{}, height{};

	SDL_Window* handle{ nullptr };
	SDL_WindowFlags windowFlags{};

	bool mouseFocus{ false };
	bool keyboardFocus{ false };
	bool shown{ false };
	bool isMinimized{ false };

	bool resizeRequested{ false };

	uint32_t nextImageIndex{};

	AllocatedImage renderTarget{};
	AllocatedImage depthTarget{};

	// TODO(piero): Temporary. This data shouldn't be here.
	AllocatedBuffer uiData{};
	AllocatedBuffer fontData{};

	// Vulkan-specific
	VkSurfaceKHR surface{ nullptr };
	PrimalSwapchain swapchain{};
};

PrimalWindow createPrimalWindow(std::string_view title, int32_t width, int32_t height, SDL_WindowFlags flags);

void destroyPrimalWindow(PrimalWindow* window, VmaAllocator& allocator, VkDevice device, VkInstance instance, VkAllocationCallbacks* callbacks);

void getWindowSize(PrimalWindow* window, int* width, int* height);
void getWindowSizeInPixels(PrimalWindow* window, int* width, int* height);

void setWindowRelativeMouseMode(PrimalWindow* window, bool enabled);

VkSurfaceKHR createVulkanSurface(PrimalWindow* window, VkInstance instance, VkAllocationCallbacks* callbacks);
void destroyVulkanSurface(VkInstance instance, VkSurfaceKHR surface, VkAllocationCallbacks* callbacks);

};// namespace pm
