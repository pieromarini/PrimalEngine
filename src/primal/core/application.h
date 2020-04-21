#pragma once

#include "log.h"
#include "core.h"

#include "input.h"
#include "window.h"
#include "layerStack.h"
#include "primal/events/event.h"
#include "primal/events/applicationEvent.h"
#include "primal/debug/instrumentor.h"

#include "timestep.h"

#include "primal/imgui/imGuiLayer.h"

namespace primal { 

  class Application {
	public:
	  Application();
	  virtual ~Application();

	  void run();

	  void onEvent(Event& e);

	  void pushLayer(Layer* layer);
	  void pushOverlay(Layer* layer);

	  inline Window& getWindow() { return *m_window; }
	  inline static Application& get() { return *s_instance; }

	private:
	  bool onWindowClose(WindowCloseEvent& e);
	  bool onWindowResize(WindowResizeEvent& e);

	  scope_ptr<Window> m_window;
	  ImGuiLayer* m_imGuiLayer;
	  bool m_running = true;
	  bool m_minimized = false;
	  LayerStack m_layerStack;
	  float m_lastFrameTime = 0.0f;

	  static Application* s_instance;
  };

  Application* createApplication();
}
