#pragma once

#include "camera.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/window.h"

namespace pm {

struct PrimalEngine {
	PrimalEngine();
	~PrimalEngine();

	void run();

	void render(f32 deltaTime, UI_EventList* events);

	void handleWindowEvent(SDL_Event& e, f32 deltaTime);

	PrimalWindow* openWindow(std::string_view name, int32_t width, int32_t height, SDL_WindowFlags flags);
	void closeWindow(PrimalWindow* window);

	void initThreadContext();

	static PrimalEngine& get();

	Arena* arena;
	PrimalWindow* firstWindow{};
	PrimalWindow* lastWindow{};

	PrimalWindow* mainWindow{};

	VulkanRendererContext rendererContext;
	bool m_isInitialized{ false };
	int m_frameNumber{ 0 };
	VkExtent2D m_windowExtent{ 2048, 1080 };
	VulkanRendererConfig m_rendererState{};

	bool windowRelativeMouseMode{ false };

	Camera m_mainCamera{ m_windowExtent.width, m_windowExtent.height };

	bool quitRequested{ false };
};

}// namespace pm
