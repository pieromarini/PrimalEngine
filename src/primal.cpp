#include <chrono>
#include <thread>

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

  constexpr unsigned long long int TIME_STEP = 1;

  const auto step = Time::step(TIME_STEP);
  const float step_ns = static_cast<float>(step.count() * 1000000);
  auto lag = Time::lag(step.count());
  auto t0 = Time::now();

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

    while (lag >= step) {
        lag -= step;
    }
    auto alpha = (float) lag.count() / step_ns;
		draw(alpha);

    // update lag and current time
    lag += Time::delta(t0);
    t0 = Time::now();

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_rendererState.rendererStats.frametime = static_cast<float>(elapsed.count());
	}
}

void PrimalApp::draw(float deltaTime) {
	m_renderer.draw(deltaTime);
}

GPUMeshBuffers PrimalApp::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices) {
	return m_renderer.uploadMesh(indices, vertices);
}

}// namespace pm
