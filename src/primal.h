#pragma once

#include "camera.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "vk_types.h"
#include "window.h"

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
	VkExtent2D m_windowExtent{ 2048, 1080 };
	VulkanRenderer m_renderer;
	VulkanRendererConfig m_rendererState{};

	bool windowRelativeMouseMode{ true };

	PrimalWindow m_window;
  std::shared_ptr<Camera> m_mainCamera;
};

}// namespace pm
