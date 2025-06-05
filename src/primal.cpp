#include <chrono>
#include <format>
#include <thread>


#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_vulkan.h>

#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_video.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "primal.h"


namespace pm {

static PrimalEngine* loadedEngine = nullptr;

PrimalEngine& PrimalEngine::get() {
	return *loadedEngine;
}

PrimalEngine::PrimalEngine() {
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	SDL_Init(SDL_INIT_VIDEO);

	rendererInit(&rendererContext);

	auto windowFlags = static_cast<SDL_WindowFlags>(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	// TODO(piero): We are reserving 20 windows because right now we won't have more than this and since we store a pointer to
	//              our main window, we don't want it to be invalidated when the vector reallocates for a resize.
	windows.reserve(20);

	mainWindow = createWindow("Primal Engine", static_cast<int32_t>(m_windowExtent.width), static_cast<int32_t>(m_windowExtent.height), windowFlags);

	// test create secondary window:
	createWindow("test window", 400, 800, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	setWindowRelativeMouseMode(mainWindow, windowRelativeMouseMode);

	// setup main viewer camera
	m_mainCamera.velocity = glm::vec3(0.f);
	m_mainCamera.position = glm::vec3(2.48f, 21.17f, 16.55f);
	m_mainCamera.yaw = -4.61;
	m_mainCamera.pitch = -0.024;
	m_mainCamera.setMouseControlEnabled(windowRelativeMouseMode);

	m_rendererState = {
		.window = mainWindow,
		.mainCamera = &m_mainCamera
	};

	// TODO(piero): Rework the initialization flow. Looks very yanky right now.
	//              We want to cleanly initialize Vulkan (aka: get an instance, device and physical device)
	//              Then we want to initialize our camera and setup all our initial "Windows".
	//              Last we create all necessary resources for our renderer (sync stuff, commands, render targets, pipelines, etc)
	rendererSetInitialState(&rendererContext, &m_rendererState);
	rendererSetup(&rendererContext);

	m_isInitialized = true;
}

void PrimalEngine::cleanup() {
	if (m_isInitialized) {
		rendererCleanup(&rendererContext);
	}
	loadedEngine = nullptr;
}

void PrimalEngine::handleWindowEvent(SDL_Event& e) {
	for (auto& window : windows) {
		if (e.window.windowID == window.id) {
			switch (e.type) {
			// Window appeared
			case SDL_EVENT_WINDOW_SHOWN:
				window.shown = true;
				break;

			// Window disappeared
			case SDL_EVENT_WINDOW_HIDDEN:
				window.shown = false;
				break;

			// Get new dimensions and repaint
			case SDL_EVENT_WINDOW_RESIZED:
				window.resizeRequested = true;
				break;

			// Repaint on expose
			case SDL_EVENT_WINDOW_EXPOSED:
				// TODO(piero): re-render window? When do we need this?
				break;

			// Mouse enter
			case SDL_EVENT_WINDOW_MOUSE_ENTER:
				window.mouseFocus = true;
				break;

			// Mouse exit
			case SDL_EVENT_WINDOW_MOUSE_LEAVE:
				window.mouseFocus = false;
				break;

			// Keyboard focus gained
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				window.keyboardFocus = true;
				break;

			// Keyboard focus lost
			case SDL_EVENT_WINDOW_FOCUS_LOST:
				window.keyboardFocus = false;
				break;

			// Window minimized
			case SDL_EVENT_WINDOW_MINIMIZED:
				window.isMinimized = true;
				break;

			// Window maximized
			case SDL_EVENT_WINDOW_MAXIMIZED:
				window.isMinimized = false;
				break;

			// Window restored
			case SDL_EVENT_WINDOW_RESTORED:
				window.isMinimized = false;
				break;

			// Hide on close
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				// NOTE(piero): if we close the main window, close the program.
				if (window.id == mainWindow->id) {
					quitRequested = true;
				} else {
					SDL_HideWindow(window.handle);
				}
				break;
			}
		}
	}
}

void PrimalEngine::run() {
	SDL_Event e;
	auto t0 = std::chrono::high_resolution_clock::now();

	while (!quitRequested) {
		auto start = std::chrono::system_clock::now();

		auto deltaTime = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
		t0 = std::chrono::high_resolution_clock::now();

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_EVENT_QUIT) {
				quitRequested = true;
			}

			if (e.type >= SDL_EVENT_WINDOW_FIRST && e.type <= SDL_EVENT_WINDOW_LAST) {
				handleWindowEvent(e);
			}

			// TODO(piero): fix window relative mouse mode. Should be tracked by window.
			if (e.type == SDL_EVENT_KEY_UP) {
				if (e.key.key == SDLK_ESCAPE) {
					windowRelativeMouseMode = !windowRelativeMouseMode;
					setWindowRelativeMouseMode(mainWindow, windowRelativeMouseMode);

					// Disable camera panning when relative mouse mode is disabled
					m_mainCamera.setMouseControlEnabled(windowRelativeMouseMode);
				} else if (e.key.key == SDLK_F) {
					rendererContext.fullScreen = !rendererContext.fullScreen;

					// TODO(piero): This shouldn't be here. Refactor
					resizeRenderTargets(&rendererContext);

					destroyGBuffer(&rendererContext);
					createGBuffer(&rendererContext);

					m_mainCamera.onWindowResize(mainWindow->width, mainWindow->height);
				} else if (e.key.key == SDLK_G) {// regenerate terrain
					cleanupTerrain(&rendererContext);
					terrainTest(&rendererContext);
				} else if (e.key.key == SDLK_0) {
					rendererContext.gbufferDebugChannel = 0;
				} else if (e.key.key == SDLK_1) {
					rendererContext.gbufferDebugChannel = 1;
				} else if (e.key.key == SDLK_2) {
					rendererContext.gbufferDebugChannel = 2;
				}
			}

			m_mainCamera.processSDLEvent(e);

			if (!windowRelativeMouseMode) {
				if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
					// TODO(piero): this takes any button click as preseed. Need to differentiate between L/R/M clicks.
					setPointerState(e.button.windowID, e.button.x, e.button.y, 0.0f, 0.0f, e.button.down);
				} else if (e.type == SDL_EVENT_MOUSE_MOTION) {
					setPointerState(e.motion.windowID, e.motion.x, e.motion.y, e.motion.xrel, e.motion.yrel, e.motion.state & SDL_BUTTON_LMASK);
				}
			}

			m_stopRendering = mainWindow->isMinimized;
		}

		// do not draw if we are minimized
		if (m_stopRendering) {
			// throttle the speed to avoid the endless spinning
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		// Check if we need to resize any windows
		for (auto& window : windows) {
			if (window.resizeRequested) {
				resizeSwapchain(&rendererContext, &window);

				// NOTE(piero): If we resize the main window, we also update our camera.
				if (window.id == mainWindow->id) {
					m_mainCamera.onWindowResize(window.width, window.height);
				}
			}
		}

		rendererUpdate(&rendererContext, deltaTime);
		rendererDraw(&rendererContext);

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_rendererState.rendererStats.frametime = static_cast<float>(elapsed.count());
	}
}

PrimalWindow* PrimalEngine::createWindow(std::string_view name, int32_t width, int32_t height, SDL_WindowFlags flags) {
	windows.push_back(createPrimalWindow(name, width, height, flags));
	auto& window = windows.back();
	window.surface = createVulkanSurface(&window, rendererContext.instance, nullptr);
	window.swapchain = createSwapchain(rendererContext.device, rendererContext.physicalDevice, window.surface, width, height, VK_FORMAT_B8G8R8A8_UNORM, VK_PRESENT_MODE_IMMEDIATE_KHR);
	return &window;
}

}// namespace pm
