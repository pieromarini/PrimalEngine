#include <chrono>
#include <thread>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "primal.h"

constexpr bool bUseValidationLayers = true;

namespace pm {

PrimalApp* loadedEngine = nullptr;

PrimalApp& PrimalApp::get() { return *loadedEngine; }

void PrimalApp::init() {
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	SDL_Init(SDL_INIT_VIDEO);

	auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	m_window = SDL_CreateWindow(
		"Vulkan Engine",
		static_cast<int32_t>(m_windowExtent.width),
		static_cast<int32_t>(m_windowExtent.height),
		window_flags);

	m_mainCamera = new Camera();
	m_mainCamera->velocity = glm::vec3(0.f);
	m_mainCamera->position = glm::vec3(0, 0, 5);

	m_mainCamera->pitch = 0;
	m_mainCamera->yaw = 0;

	m_rendererState = {
		.useValidationLayers = true,
		.windowExtent = m_windowExtent,
		.window = m_window,
		.mainCamera = m_mainCamera
	};

	m_renderer.init(&m_rendererState);

	m_isInitialized = true;
}

void PrimalApp::cleanup() {
	if (m_isInitialized) {
		m_renderer.cleanup();
		SDL_DestroyWindow(m_window);
	}
	loadedEngine = nullptr;
}

void PrimalApp::run() {
	SDL_Event e;
	bool bQuit = false;

	while (!bQuit) {
		auto start = std::chrono::system_clock::now();

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_EVENT_QUIT)
				bQuit = true;

			if (e.type == SDL_EVENT_WINDOW_MINIMIZED || e.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
				m_stopRendering = true;
			}
			if (e.type == SDL_EVENT_WINDOW_RESTORED || e.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
				m_stopRendering = false;
			}

			m_mainCamera->processSDLEvent(e);
		}

		// do not draw if we are minimized
		if (m_stopRendering) {
			// throttle the speed to avoid the endless spinning
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		if (m_rendererState.resizeRequested) {
			m_renderer.resizeSwapchain();
		}

		draw();

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		m_rendererState.rendererStats.frametime = elapsed.count() / 1000.0f;

		auto stats = std::format("Frametime: {}us | Update: {}us | MeshDraw: {}us | Triangles: {} | DrawCall: {}",
			m_rendererState.rendererStats.frametime,
			m_rendererState.rendererStats.sceneUpdateTime,
			m_rendererState.rendererStats.meshDrawTime,
			m_rendererState.rendererStats.triangleCount,
			m_rendererState.rendererStats.drawCallCount);
		std::cout << stats << '\n';
	}
}

void PrimalApp::draw() {
	m_renderer.draw();
}

GPUMeshBuffers PrimalApp::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
	return m_renderer.uploadMesh(indices, vertices);
}

}// namespace pm
