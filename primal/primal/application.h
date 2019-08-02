#pragma once

#include <memory>

#include "core.h"

#include "window.h"
#include "layerStack.h"
#include "events/event.h"
#include "events/applicationEvent.h"

#include "core/timestep.h"

#include "imgui/imGuiLayer.h"

namespace primal { 

  class Application {
	public:
	  Application();
	  virtual ~Application() = default;

	  void run();

	  void onEvent(Event& e);

	  void pushLayer(Layer* layer);
	  void pushOverlay(Layer* layer);

	  inline Window& getWindow() { return *m_Window; }

	  inline static Application& get() { return *s_Instance; }
	private:
	  bool onWindowClose(WindowCloseEvent& e);

	  std::unique_ptr<Window> m_Window;
	  ImGuiLayer* m_ImGuiLayer;
	  bool m_Running = true;
	  LayerStack m_LayerStack;
	  float m_LastFrameTime = 0.0f;

	  static Application* s_Instance;
  };

  Application* createApplication();
}
