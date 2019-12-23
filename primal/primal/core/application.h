#pragma once

#include "core.h"

#include "log.h"
#include "input.h"
#include "window.h"
#include "layerStack.h"
#include "../events/event.h"
#include "../events/applicationEvent.h"

#include "timestep.h"

#include "../imgui/imGuiLayer.h"

namespace primal { 

  class Application {
	public:
	  Application();
	  virtual ~Application() = default;

	  void run();

	  void onEvent(Event& e);

	  void pushLayer(Layer* layer);
	  void pushOverlay(Layer* layer);

	  inline Window& getWindow() { return *m_window; }

	  inline static Application& get() { return *s_instance; }
	private:
	  bool onWindowClose(WindowCloseEvent& e);

	  scope_ptr<Window> m_window;
	  ImGuiLayer* m_imGuiLayer;
	  bool m_running = true;
	  LayerStack m_layerStack;
	  float m_lastFrameTime = 0.0f;

	  static Application* s_instance;
  };

  Application* createApplication();
}
