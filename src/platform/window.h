#pragma once

#include <SDL3/SDL_video.h>
#include "core/core.h"
#include "platform/vulkan/swapchain.h"
#include "vk_types.h"
#include <string_view>
#include <vulkan/vulkan_core.h>

namespace pm {

struct UIContext;

// TODO(piero): Refactor panel stuff out of here
struct Panel {
	Panel* first;
	Panel* last;
	Panel* next;
	Panel* prev;
	Panel* parent;
	f32 sizePct;
	Axis2D splitAxis;
};


// Structures to easily traverse the panel tree
struct PanelTraversalStep {
	Panel* next;
	i32 pushCount;
	i32 popCount;
};

struct TraverseNode {
	TraverseNode* next;
	Panel* parent;
	Panel* child;
};

struct PrimalWindow {
	u64 id;
	
	i32 width, height;

	SDL_Window* handle;
	SDL_WindowFlags windowFlags;

	bool mouseFocus;
	bool keyboardFocus;
	bool shown;
	bool isMinimized;

	bool redraw;
	bool resizeRequested;

	uint32_t nextImageIndex;

	AllocatedImage renderTarget;
	AllocatedImage depthTarget;

	// TODO(piero): Temporary. This data shouldn't be here.
	AllocatedBuffer uiData;
	AllocatedBuffer fontData;

	// Vulkan-specific
	VkSurfaceKHR surface;
	PrimalSwapchain swapchain;

	PrimalWindow* next;
	PrimalWindow* prev;
	UIContext* uiContext;
	Arena* arena;
	Panel* rootPanel;
	Panel* freePanel;
};

PrimalWindow* createPrimalWindow(Arena* arena, std::string_view title, int32_t width, int32_t height, SDL_WindowFlags flags);

void destroyPrimalWindow(PrimalWindow* window, VmaAllocator& allocator, VkDevice device, VkInstance instance, VkAllocationCallbacks* callbacks);

void getWindowSize(PrimalWindow* window, int* width, int* height);
void getWindowSizeInPixels(PrimalWindow* window, int* width, int* height);

void setWindowRelativeMouseMode(PrimalWindow* window, bool enabled);

VkSurfaceKHR createVulkanSurface(PrimalWindow* window, VkInstance instance, VkAllocationCallbacks* callbacks);
void destroyVulkanSurface(VkInstance instance, VkSurfaceKHR surface, VkAllocationCallbacks* callbacks);

// Panel helpers
PanelTraversalStep depthFirstPreOrderStep(Panel* panel);
Rect2D rectFromPanelChild(Panel* child, Rect2D parentRect);
Rect2D rectFromPanel(Panel *panel, Rect2D rootRect);

};// namespace pm
