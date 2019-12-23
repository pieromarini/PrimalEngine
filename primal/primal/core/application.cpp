#include <GLFW/glfw3.h>

#include "application.h"

#include "../renderer/renderer.h"

namespace primal {

#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

  Application* Application::s_instance = nullptr;

  Application::Application() {
	PRIMAL_CORE_ASSERT(!s_instance, "Application already exists!");
	s_instance = this;

	m_window = scope_ptr<Window>(Window::create());
	m_window->setEventCallback(BIND_EVENT_FN(onEvent));

	m_imGuiLayer = new ImGuiLayer();
	pushOverlay(m_imGuiLayer);
  }

  void Application::pushLayer(Layer* layer) {
	m_layerStack.pushLayer(layer);
  }

  void Application::pushOverlay(Layer* layer) {
	m_layerStack.pushOverlay(layer);
  }

  void Application::onEvent(Event& e) {
	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowCloseEvent>(BIND_EVENT_FN(onWindowClose));

	for (auto it = m_layerStack.end(); it != m_layerStack.begin();) {
	  (*--it)->onEvent(e);
	  if (e.Handled)
		break;
	}
  }

  void Application::run() {
	while (m_running) {
	  // NOTE: Application shouldn't be tied to GLFW. Move to platform
	  float time = static_cast<float>(glfwGetTime()); 
	  Timestep timestep = time - m_lastFrameTime;
	  m_lastFrameTime = time;

	  for (Layer* layer : m_layerStack)
		layer->onUpdate(timestep);

	  m_imGuiLayer->begin();
	  for (Layer* layer : m_layerStack)
		layer->onImGuiRender();
	  m_imGuiLayer->end();

	  m_window->onUpdate();
	}
  }

  bool Application::onWindowClose(WindowCloseEvent& e) {
	m_running = false;
	return true;
  }

}
