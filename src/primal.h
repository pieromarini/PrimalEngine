#pragma once

#include "camera.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "vk_types.h"

namespace pm {

struct Time {
	// get current time
	static auto now() {
		return std::chrono::high_resolution_clock::now();
	}

	// get t as milliseconds
	static auto step(const unsigned long long int t) {
		return std::chrono::milliseconds(t);
	}

	// get t as nanoseconds
	static auto lag(const unsigned long long int t) {
		using namespace std::chrono_literals;
		return std::chrono::nanoseconds(0ns) + step(t);
	}

	// get delta time
	static auto delta(const std::chrono::time_point<std::chrono::high_resolution_clock>& t0) {
		return duration_cast<std::chrono::nanoseconds>(now() - t0);
	}
};

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
	Camera* m_mainCamera;
};

}// namespace pm
