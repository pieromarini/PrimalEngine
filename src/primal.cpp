#include <chrono>
#include <thread>
#include <format>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_vulkan.h>

#include "primal.h"

constexpr bool bUseValidationLayers = true;

namespace pm {

PrimalApp* loadedEngine = nullptr;

PrimalApp& PrimalApp::get() { return *loadedEngine; }

PrimalApp::PrimalApp() {
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	SDL_Init(SDL_INIT_VIDEO);

	auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	m_window = SDL_CreateWindow(
		"Primal Engine",
		static_cast<int32_t>(m_windowExtent.width),
		static_cast<int32_t>(m_windowExtent.height),
		window_flags);

	SDL_SetWindowRelativeMouseMode(m_window, true);

	m_mainCamera = std::make_shared<Camera>(m_windowExtent.width, m_windowExtent.height);
	m_mainCamera->velocity = glm::vec3(0.f);
	m_mainCamera->position = glm::vec3(-15.f, 3.5f, -1.1f);
	m_mainCamera->yaw = -4.61;
	m_mainCamera->pitch = -0.024;

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

	auto t0 = std::chrono::high_resolution_clock::now();

	while (!bQuit) {
		auto start = std::chrono::system_clock::now();

		auto deltaTime = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
		t0 = std::chrono::high_resolution_clock::now();

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
			m_mainCamera->onWindowResize(m_rendererState.windowExtent.width, m_rendererState.windowExtent.height);
		}

		draw(deltaTime);

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_rendererState.rendererStats.frametime = static_cast<float>(elapsed.count());
	}
}

void PrimalApp::draw(float deltaTime) {
	m_renderer.draw(deltaTime);
}

GPUMeshBuffers PrimalApp::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
	return m_renderer.uploadMesh(indices, vertices, "PrimalEngine");
}

}// namespace pm
