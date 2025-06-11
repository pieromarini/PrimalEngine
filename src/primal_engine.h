#pragma once

#include "camera.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/window.h"


namespace pm {

class PrimalEngine {
public:
	PrimalEngine();
	void run();
	void cleanup();
	void handleWindowEvent(SDL_Event& e);
	PrimalWindow* createWindow(std::string_view name, int32_t width, int32_t height, SDL_WindowFlags flags);
	static PrimalEngine& get();

	PrimalWindow* firstWindow{};
	PrimalWindow* lastWindow{};

	Arena* arena;

	VulkanRendererContext rendererContext;
private:
	void initThreadContext();
	bool m_isInitialized{ false };
	int m_frameNumber{ 0 };
	bool m_stopRendering{ false };
	VkExtent2D m_windowExtent{ 2048, 1080 };
	VulkanRendererConfig m_rendererState{};

	bool windowRelativeMouseMode{ false };

	Camera m_mainCamera{ m_windowExtent.width, m_windowExtent.height };

	bool quitRequested{ false };
};

}// namespace pm
