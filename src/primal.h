#pragma once

#include "vk_types.h"
#include "platform/vulkan/vulkan_renderer.h"

namespace pm {

class PrimalApp {
	public:
		void init();
		void run();
		void draw();
		void cleanup();
		PrimalApp& get();

		GPUMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);

	private:
		bool m_isInitialized{ false };
		int m_frameNumber{ 0 };
		bool m_stopRendering{ false };
		VkExtent2D m_windowExtent{ 1700, 900 };
		VulkanRenderer m_renderer{};

		SDL_Window* m_window{ nullptr };
};

}// namespace pm
