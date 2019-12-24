#include <GLFW/glfw3.h>

#include "application.h"

#include "../renderer/renderer.h"

namespace primal {

  Application* Application::s_instance = nullptr;

  Application::Application() {
	PRIMAL_CORE_ASSERT(!s_instance, "Application already exists!");
	s_instance = this;

	m_window = scope_ptr<Window>(Window::create());
	m_window->setEventCallback(PRIMAL_BIND_EVENT_FN(Application::onEvent));

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
	dispatcher.dispatch<WindowCloseEvent>(PRIMAL_BIND_EVENT_FN(Application::onWindowClose));
	dispatcher.dispatch<WindowResizeEvent>(PRIMAL_BIND_EVENT_FN(Application::onWindowResize));

	for (auto it = m_layerStack.end(); it != m_layerStack.begin();) {
	  (*--it)->onEvent(e);
	  if (e.Handled)
		break;
	}
  }

  void Application::run() {
	while (m_running) {
	  float time = static_cast<float>(glfwGetTime());
	  Timestep timestep = time - m_lastFrameTime;
	  m_lastFrameTime = time;

	  if (!m_minimized) {
		for (Layer* layer : m_layerStack)
		  layer->onUpdate(timestep);
	  }

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

  bool Application::onWindowResize(WindowResizeEvent& e) {
	if (e.getWidth() == 0 || e.getHeight() == 0) {
	  m_minimized = true;
	  return false;
	}

	m_minimized = false;
	Renderer::onWindowResize(e.getWidth(), e.getHeight());

	return false;
  }

  Application::~Application() {
	Renderer::shutdown();
  }

}
