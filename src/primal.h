#pragma once

#include "camera.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "vk_types.h"
#include "platform/window.h"

namespace pm {

class PrimalEngine {
public:
	PrimalEngine();
	void run();
	void draw(float deltaTime);
	void cleanup();
	void handleWindowEvent(SDL_Event& e);
	PrimalWindow* createWindow(std::string_view name, int32_t width, int32_t height, SDL_WindowFlags flags);
	static PrimalEngine& get();

	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

	std::vector<PrimalWindow> windows;

private:
	bool m_isInitialized{ false };
	int m_frameNumber{ 0 };
	bool m_stopRendering{ false };
	VkExtent2D m_windowExtent{ 2048, 1080 };
	VulkanRenderer m_renderer;
	VulkanRendererConfig m_rendererState{};

	bool windowRelativeMouseMode{ true };

	PrimalWindow* mainWindow;
  std::shared_ptr<Camera> m_mainCamera;

	bool quitRequested{ false };
};

}// namespace pm
