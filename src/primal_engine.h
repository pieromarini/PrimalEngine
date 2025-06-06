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

	std::vector<PrimalWindow> windows;

private:
	bool m_isInitialized{ false };
	int m_frameNumber{ 0 };
	bool m_stopRendering{ false };
	VkExtent2D m_windowExtent{ 2048, 1080 };
	VulkanRendererConfig m_rendererState{};

	bool windowRelativeMouseMode{ false };

	PrimalWindow* mainWindow;
	Camera m_mainCamera{ m_windowExtent.width, m_windowExtent.height };

	bool quitRequested{ false };

	VulkanRendererContext rendererContext;
};

}// namespace pm
