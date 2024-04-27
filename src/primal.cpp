#include <chrono>
#include <thread>

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "primal.h"

constexpr bool bUseValidationLayers = true;

pm::PrimalApp* loadedEngine = nullptr;

pm::PrimalApp& pm::PrimalApp::get() { return *loadedEngine; }

void pm::PrimalApp::init() {
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	SDL_Init(SDL_INIT_VIDEO);

	auto window_flags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN);

	m_window = SDL_CreateWindow(
		"Vulkan Engine",
		static_cast<int32_t>(m_windowExtent.width),
		static_cast<int32_t>(m_windowExtent.height),
		window_flags);

	VulkanRendererConfig config = {
		.useValidationLayers = true,
		.windowExtent = m_windowExtent,
		.window = m_window
	};

	m_renderer.init(config);

	m_isInitialized = true;
}

void pm::PrimalApp::cleanup() {
	if (m_isInitialized) {
		m_renderer.cleanup();
		SDL_DestroyWindow(m_window);
	}
	loadedEngine = nullptr;
}

void pm::PrimalApp::run() {
	SDL_Event e;
	bool bQuit = false;

	while (!bQuit) {
		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_EVENT_QUIT)
				bQuit = true;

			if (e.type == SDL_EVENT_WINDOW_MINIMIZED || e.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
				m_stopRendering = true;
			}
			if (e.type == SDL_EVENT_WINDOW_RESTORED || e.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
				m_stopRendering = false;
			}
		}

		// do not draw if we are minimized
		if (m_stopRendering) {
			// throttle the speed to avoid the endless spinning
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		draw();
	}
}

void pm::PrimalApp::draw() {
	m_renderer.draw();
}
