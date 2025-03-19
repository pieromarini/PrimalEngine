#pragma once

#include "camera.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "vk_types.h"

namespace pm {

class PrimalApp {
public:
	PrimalApp();
	void run();
	void draw(float deltaTime);
	void cleanup();
	PrimalApp& get();

	GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

private:
	bool m_isInitialized{ false };
	int m_frameNumber{ 0 };
	bool m_stopRendering{ false };
	VkExtent2D m_windowExtent{ 1920, 1080 };
	VulkanRenderer m_renderer;
	VulkanRendererConfig m_rendererState{};

	SDL_Window* m_window{ nullptr };
  std::shared_ptr<Camera> m_mainCamera;
};

}// namespace pm
