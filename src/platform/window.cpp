#include "window.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_vulkan.h"

#include "vulkan/buffers.h"

namespace pm {

PrimalWindow* createPrimalWindow(Arena* arena, std::string_view title, int32_t width, int32_t height, SDL_WindowFlags flags) {
	auto window = PushStruct(arena, PrimalWindow);
	window->width = width;
	window->height = height;
	window->windowFlags = flags;
	window->mouseFocus = false;
	window->keyboardFocus = false;
	window->shown = false;
	window->isMinimized = false;

	window->handle = SDL_CreateWindow(title.data(), width, height, flags);

	window->id = SDL_GetWindowID(window->handle);

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

PanelTraversalStep depthFirstPreOrderStep(Panel* panel) {
	PanelTraversalStep rec{};

	if (panel->first != nullptr) {
		// Panel has child
		rec.next = panel->first;
		rec.pushCount = 1;
	} else {
		// Left node. Traverse up until we find a node with a sibling
		for (auto* p = panel; p != nullptr; p = p->parent) {
			if (p->next != nullptr) {
				rec.next = p->next;
				break;
			}
			rec.popCount++;
		}
	}

	return rec;
}

Rect2D rectFromPanelChild(Panel* child, Rect2D parentRect) {
	Rect2D result = parentRect;

	auto* parent = child->parent;

	if (parent != nullptr) {
		auto parentRectDim = rect2DSize(parentRect);
		result.max[parent->splitAxis] = result.min[parent->splitAxis];
		for (auto* p = parent->first; p != child && p != nullptr; p = p->next) {
			result.min[parent->splitAxis] += p->sizePct * parentRectDim[parent->splitAxis];
			result.max[parent->splitAxis] = result.min[parent->splitAxis];
		}
		result.max[parent->splitAxis] += child->sizePct * parentRectDim[parent->splitAxis];
	}

	return result;
}

Rect2D rectFromPanel(Panel* panel, Rect2D rootRect) {
	auto scratch = ScratchBegin();

	TraverseNode* travNode = nullptr;
	for (Panel* p = panel; p != nullptr && p->parent != nullptr; p = p->parent) {
		auto* node = PushStruct(scratch.arena, TraverseNode);
		node->parent = panel->parent;
		node->child = panel;
		StackPush(travNode, node);
	}

	auto result = rootRect;
	for (TraverseNode* node = travNode; node != nullptr; node = node->next) {
		result = rectFromPanelChild(node->child, result);
	}

	ScratchEnd(scratch);

	return result;
}

}// namespace pm
