#include <chrono>
#include <thread>
#include <cinttypes>

#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_vulkan.h>


#include "SDL3/SDL_events.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_video.h"
#include "core/math/math.h"
#include "core/memory/arena.h"
#include "core/thread_context.h"
#include "platform/os/keys.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "platform/window.h"
#include "primal_engine.h"
#include "ui/generated.h"
#include "ui/ui_manager.h"
#include "ui/ui_types.h"


namespace pm {

static PrimalEngine* loadedEngine = nullptr;

PrimalEngine& PrimalEngine::get() {
	return *loadedEngine;
}

PrimalEngine::PrimalEngine() {
	assert(loadedEngine == nullptr);
	loadedEngine = this;

	initThreadContext();

	arena = arenaAlloc(Gigabytes(4));

	SDL_Init(SDL_INIT_VIDEO);
	rendererInit(&rendererContext);

	auto windowFlags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

	mainWindow = openWindow("Primal Engine", (i32)m_windowExtent.width, (i32)m_windowExtent.height, windowFlags);

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

	rendererSetInitialState(&rendererContext, &m_rendererState);
	rendererSetup(&rendererContext);

	m_isInitialized = true;
}

PrimalEngine::~PrimalEngine() {
	if (m_isInitialized) {
		rendererCleanup(&rendererContext);
		arenaRelease(arena);
	}

	ThreadCtx_release();

	loadedEngine = nullptr;
}

void PrimalEngine::handleWindowEvent(SDL_Event& e, f32 deltaTime) {
	for (auto* window = firstWindow; window != nullptr; window = window->next) {
		if (e.window.windowID == window->id) {
			switch (e.type) {
			// Window appeared
			case SDL_EVENT_WINDOW_SHOWN:
				window->shown = true;
				break;

			// Window disappeared
			case SDL_EVENT_WINDOW_HIDDEN:
				window->shown = false;
				break;

			case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				// render(deltaTime);
				break;

			// Get new dimensions and repaint
			case SDL_EVENT_WINDOW_RESIZED:
				window->resizeRequested = true;
				break;

			// Repaint on expose
			case SDL_EVENT_WINDOW_EXPOSED:
				// update(deltaTime);
				break;

			// Mouse enter
			case SDL_EVENT_WINDOW_MOUSE_ENTER:
				window->mouseFocus = true;
				break;

			// Mouse exit
			case SDL_EVENT_WINDOW_MOUSE_LEAVE:
				window->mouseFocus = false;
				break;

			// Keyboard focus gained
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				window->keyboardFocus = true;
				break;

			// Keyboard focus lost
			case SDL_EVENT_WINDOW_FOCUS_LOST:
				window->keyboardFocus = false;
				break;

			// Window minimized
			case SDL_EVENT_WINDOW_MINIMIZED:
				window->isMinimized = true;
				break;

			// Window maximized
			case SDL_EVENT_WINDOW_MAXIMIZED:
				window->isMinimized = false;
				break;

			// Window restored
			case SDL_EVENT_WINDOW_RESTORED:
				window->isMinimized = false;
				break;

			// Hide on close
			case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
				// NOTE(piero): if we close the main window, close the program.
				closeWindow(window);
				break;
			}
		}
	}
}

void PrimalEngine::render(f32 deltaTime, UI_EventList* events) {

	auto scratch = ScratchBegin();

	auto rendererState = rendererContext.rendererState;

	auto stats = PushStr8F(scratch.arena, "Frametime: %.2fms | GPU: %.2fms | UISubmit: %.4fms | UI: %.4fms | Triangles: %.2fM | DrawCall: %" PRIu32 "",
		rendererState->rendererStats.frametime,
		rendererState->rendererStats.frameGpuTimeAvg,
		rendererState->rendererStats.renderSubmitTimeAvg,
		rendererState->rendererStats.uiRenderTimeAvg,
		rendererState->rendererStats.triangleCount * 1e-6,
		rendererState->rendererStats.drawCallCount);

	auto otherStats = PushStr8F(scratch.arena, "DrawBatchGen: %.4fus | EntityFlatten: %.4fus | UISetupBuffers: %.4fus | UIBuildTime: %.4fus | SceneUpdate: %.4fus | MeshDraw: %.4fus",
		rendererState->rendererStats.drawBatchGenerationTimeAvg,
		rendererState->rendererStats.entityFlattenTimeAvg,
		rendererState->rendererStats.uiSetupBuffersTimeAvg,
		rendererState->rendererStats.uiBuildTimeAvg,
		rendererState->rendererStats.sceneUpdateTimeAvg,
		rendererState->rendererStats.meshDrawTimeAvg);

	auto buildStart = std::chrono::high_resolution_clock::now();
	Renderer_beginFrame(&rendererContext);

	for (PrimalWindow* window = firstWindow; window != nullptr; window = window->next) {
		Renderer_beginWindow(&rendererContext, window);

		// do not render if we are minimized
		if (window->isMinimized) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		// Check if we need to resize
		if (window->resizeRequested) {
			resizeSwapchain(&rendererContext, window);

			// NOTE(piero): If we resize the main window, we also update our camera.
			if (window->id == mainWindow->id) {
				m_mainCamera.onWindowResize(window->width, window->height);
			}
		}

		UI_setCurrentContext(window->uiContext);

		UI_beginBuild(window, events, deltaTime);

		UI_pushFont(&rendererContext.sourceCodeFont);
		UI_pushFontSize(16.0f);

		Rect2D windowRect = { .min = { 0, 0 }, .max = { window->width, window->height } };
		auto windowSize = rect2DSize(windowRect);

		for (auto* panel = window->rootPanel; panel != nullptr; panel = depthFirstPreOrderStep(panel).next) {
			auto panelRect = rectFromPanel(panel, windowRect);
			auto panelRectDim = rect2DSize(panelRect);

			for (auto* child = panel->first; child != nullptr && child->next != nullptr; child = child->next) {
				auto childRect = rectFromPanelChild(child, panelRect);
				Rect2D boundaryRect = childRect;

				boundaryRect.min[panel->splitAxis] = boundaryRect.max[panel->splitAxis];
				boundaryRect.min[panel->splitAxis] -= 2;
				boundaryRect.max[panel->splitAxis] += 2;

				UI_setNextFixedRect(boundaryRect);
				auto boundaryElement = UIElement_create(UIElementFlag_Clickable | UIElementFlag_Floating, "###panel_boundary_%p", child);
				auto sig = UI_signalFromElement(boundaryElement);

				if (sig.dragging_left) {
					auto* minChild = child;
					auto* maxChild = child->next;

					if (sig.pressed_left) {
						vec2 dragData = { minChild->sizePct, maxChild->sizePct };
						UI_storeDragData(dragData);
					}
					auto dragData = UI_loadDragData();
					auto dragDelta = UI_dragDelta();

					f32 minChildPctPreDrag = dragData.x;
					f32 maxChildPctPreDrag = dragData.y;

					f32 minChildPxPreDrag = minChildPctPreDrag * panelRectDim[panel->splitAxis];
					f32 maxChildPxPreDrag = maxChildPctPreDrag * panelRectDim[panel->splitAxis];

					f32 minChildPxPostDrag = minChildPxPreDrag + dragDelta[panel->splitAxis];
					f32 maxChildPxPostDrag = maxChildPxPreDrag - dragDelta[panel->splitAxis];

					f32 minChildPctPostDrag = minChildPxPostDrag / panelRectDim[panel->splitAxis];
					f32 maxChildPctPostDrag = maxChildPxPostDrag / panelRectDim[panel->splitAxis];
					minChild->sizePct = minChildPctPostDrag;
					maxChild->sizePct = maxChildPctPostDrag;
				}
			}
		}

		for (auto* panel = window->rootPanel; panel != nullptr; panel = depthFirstPreOrderStep(panel).next) {
			auto panelRect = rectFromPanel(panel, windowRect);
			if (panel->first == nullptr) {
				UI_setNextFixedRect(rect2DPad(panelRect, -2.0f));
				UI_setNextChildLayoutAxis(Axis2D_Y);
				UIElement* panelElement = UIElement_create(UIElementFlag_DrawBorder | UIElementFlag_DrawBackground | UIElementFlag_Clickable | UIElementFlag_Floating, 
						"###panel_element_%p",
						panel);

				UI_parent(panelElement) UI_seedKey(panelElement->key) {

					UI_setNextPrefWidth(UI_Pct(1.0f, 1.0f));
					UI_setNextPrefHeight(UI_SizeByChildren(1.0f));
					UI_setNextChildLayoutAxis(Axis2D_Y);
					UIElement* statsContainer = UIElement_create(UIElementFlag_DrawBorder | UIElementFlag_DrawBackground, "###stats_container_%p", rendererContext.rendererState->mainCamera);

					UI_parent(statsContainer) UI_seedKey(statsContainer->key)
					UI_prefWidth(UI_Pct(1.0f, 1.0f)) UI_prefHeight(UI_TextDim(1.0f))
					UI_textColor((vec4{ 1.0f, 1.0f, 1.0f, 1.0f })) UI_textEdgePadding(10.0f) {
						UI_Spacer(UI_Em(5.0f, 1.0f));
						UI_Label(stats);
						UI_Label(otherStats);
						UI_Spacer(UI_Em(5.0f, 1.0f));
					}

					UI_setNextTextEdgePadding(50.0f);
					UI_setNextTextAlignment(UITextAlignment_Center);
					UI_setNextBorderColor({ 1.0f, 0.0f, 0.0f, 0.6f });
					UI_setNextBorderThickness(3.0f);
					UI_setNextCornerRadius(20.0f);
					UI_setNextPrefWidth(UI_Pct(0.2f, 1.0f));
					UI_setNextPrefHeight(UI_Pixels(80.0f, 1.0f));
					UI_setNextBackgroundColor({ 0.0f, 1.0f, 0.0f, 1.0f });
					if (UI_Button(Str8L("Button 1")).clicked_left) {
						std::cout << "Clicked button 1\n";
					}

					UI_setNextTextColor({ 1.0f, 0.0f, 0.0f, 1.0f });
					UI_setNextBackgroundColor({ 0.26f, 0.29f, 0.31f, 1.0f });
					UI_setNextCornerRadius(5.0f);
					UI_setNextPrefWidth(UI_TextDim(1.0f));
					UI_setNextPrefHeight(UI_Pixels(80.0f, 1.0f));
					UI_setNextTextEdgePadding(10.0f);
					if (UI_Button(Str8L("Button 2")).clicked_left) {
						std::cout << "Clicked button 2\n";
					}
				}
			}
		}

		UI_endBuild();

		UI_draw(&rendererContext);

		Renderer_endWindow(&rendererContext);

		ScratchEnd(scratch);
	}

	auto buildTime = std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - buildStart).count();
	rendererContext.rendererState->rendererStats.uiBuildTimeAvg = rendererContext.rendererState->rendererStats.uiBuildTimeAvg * 0.95 + buildTime * 0.05;


	auto setupBuffersStart = std::chrono::high_resolution_clock::now();

	Renderer_setupBuffers(&rendererContext);

	auto setupBuffersTime = std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - setupBuffersStart).count();
	rendererContext.rendererState->rendererStats.uiSetupBuffersTimeAvg = rendererContext.rendererState->rendererStats.uiSetupBuffersTimeAvg * 0.95 + setupBuffersTime * 0.05;


	auto renderSubmitStart = std::chrono::high_resolution_clock::now();

	Renderer_draw(&rendererContext);
	Renderer_endFrame(&rendererContext);

	auto renderSubmitTime = std::chrono::duration<double, std::micro>(std::chrono::high_resolution_clock::now() - renderSubmitStart).count();
	rendererContext.rendererState->rendererStats.renderSubmitTimeAvg = rendererContext.rendererState->rendererStats.renderSubmitTimeAvg * 0.95 + (renderSubmitTime * 1e-3)  * 0.05;
}

void PrimalEngine::run() {
	SDL_Event e;
	auto t0 = std::chrono::high_resolution_clock::now();

	while (!quitRequested) {

		auto start = std::chrono::system_clock::now();
		auto deltaTime = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
		t0 = std::chrono::high_resolution_clock::now();

		auto scratch = ScratchBegin();

		auto events = PushStruct(scratch.arena, UI_EventList);

		while (SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_EVENT_QUIT) {
				quitRequested = true;
			}

			if (e.type >= SDL_EVENT_WINDOW_FIRST && e.type <= SDL_EVENT_WINDOW_LAST) {
				handleWindowEvent(e, deltaTime);
			}

			if (e.type >= SDL_EVENT_MOUSE_MOTION && e.type <= SDL_EVENT_MOUSE_WHEEL) {
				auto eventNode = PushStruct(scratch.arena, UI_EventNode);
				eventNode->v = {
					.kind = sdlEventTypeToUIEventKind((SDL_EventType)e.type),
				};
				if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
					eventNode->v.position = { e.button.x, e.button.y };
					eventNode->v.delta = { 0.0f, 0.0f };
					eventNode->v.key = OS_Key_MouseLeft;
				} else if (e.type == SDL_EVENT_MOUSE_MOTION) {
					eventNode->v.position = { e.motion.x, e.motion.y };
					eventNode->v.delta = { e.motion.xrel, e.motion.xrel };
					eventNode->v.key = e.motion.state & SDL_BUTTON_LMASK
																			? OS_Key_MouseLeft
																			: (e.motion.state & SDL_BUTTON_RMASK
																					? OS_Key_MouseRight
																					: OS_Key_MouseMiddle);
				}
				DLLPushBack(events->first, events->last, eventNode);
				events->count++;
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
				} else if (e.key.key == SDLK_P) {
					openWindow("Window", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
				}
			}

			m_mainCamera.processSDLEvent(e);
		}

		Renderer_update(&rendererContext, deltaTime);

		render(deltaTime, events);

		auto end = std::chrono::system_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		m_rendererState.rendererStats.frametime = static_cast<float>(elapsed.count());

		ScratchEnd(scratch);
	}

}

PrimalWindow* PrimalEngine::openWindow(std::string_view name, int32_t width, int32_t height, SDL_WindowFlags flags) {
	auto* window = createPrimalWindow(arena, name, width, height, flags);

	window->surface = createVulkanSurface(window, rendererContext.instance, nullptr);
	window->swapchain = createSwapchain(rendererContext.device, rendererContext.physicalDevice, window->surface, width, height, VK_FORMAT_B8G8R8A8_UNORM, VK_PRESENT_MODE_IMMEDIATE_KHR);

	window->uiContext = UI_createContext();
	window->arena = arenaAlloc(Gigabytes(2));
	window->rootPanel = PushStruct(window->arena, Panel);
	window->rootPanel->sizePct = 1.0f;
	window->rootPanel->splitAxis = Axis2D_X;

	Renderer_initWindow(&rendererContext, window);

	auto left = PushStruct(window->arena, Panel);
	auto right = PushStruct(window->arena, Panel);
	left->splitAxis = Axis2D_Y;
	left->sizePct = right->sizePct = 0.5f;

	left->parent = right->parent = window->rootPanel;
	DLLPushBack(window->rootPanel->first, window->rootPanel->last, left);
	DLLPushBack(window->rootPanel->first, window->rootPanel->last, right);

	DLLPushBack(firstWindow, lastWindow, window);

	return window;
}

void PrimalEngine::closeWindow(PrimalWindow* window) {
	// NOTE(piero): Close the app if the main window is closed.
	if (window->id == mainWindow->id) {
		quitRequested = true;
		return;
	}

	DLLRemove(firstWindow, lastWindow, window);
	UI_destroyContext(window->uiContext);
	destroyPrimalWindow(window, rendererContext.vmaAllocator, rendererContext.device, rendererContext.instance, nullptr);
	arenaRelease(window->arena);
}

void PrimalEngine::initThreadContext() {
	static auto tctx = ThreadCtx_alloc();
	ThreadCtx_set(&tctx);
}

}// namespace pm
