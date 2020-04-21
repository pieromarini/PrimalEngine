#include <GLFW/glfw3.h>

#include "application.h"

#include "primal/renderer/renderer.h"

namespace primal {

  Application* Application::s_instance = nullptr;

  Application::Application() {

	PRIMAL_PROFILE_FUNCTION();

	PRIMAL_CORE_ASSERT(!s_instance, "Application already exists!");
	s_instance = this;

	m_window = Window::create();
	m_window->setEventCallback(PRIMAL_BIND_EVENT_FN(Application::onEvent));

	Renderer::init();

	m_imGuiLayer = new ImGuiLayer();
	pushOverlay(m_imGuiLayer);
  }

  Application::~Application() {
	PRIMAL_PROFILE_FUNCTION();

	Renderer::shutdown();
  }

  void Application::pushLayer(Layer* layer) {
	PRIMAL_PROFILE_FUNCTION();
	m_layerStack.pushLayer(layer);
	layer->onAttach();
  }

  void Application::pushOverlay(Layer* layer) {
	PRIMAL_PROFILE_FUNCTION();
	m_layerStack.pushOverlay(layer);
	layer->onAttach();
  }

  void Application::onEvent(Event& e) {
	PRIMAL_PROFILE_FUNCTION();
	EventDispatcher dispatcher{e};
	dispatcher.dispatch<WindowCloseEvent>(PRIMAL_BIND_EVENT_FN(Application::onWindowClose));
	dispatcher.dispatch<WindowResizeEvent>(PRIMAL_BIND_EVENT_FN(Application::onWindowResize));

	for (auto it = m_layerStack.rbegin(); it != m_layerStack.rend(); ++it) {
	  (*it)->onEvent(e);
	  if (e.handled)
		break;
	}
  }

  void Application::run() {
	PRIMAL_PROFILE_FUNCTION();

	while (m_running) {
	  PRIMAL_PROFILE_SCOPE("ApplicationLoop");
	  float time = static_cast<float>(glfwGetTime());
	  Timestep timestep = time - m_lastFrameTime;
	  m_lastFrameTime = time;

	  if (!m_minimized) {
		{
		  PRIMAL_PROFILE_SCOPE("LayerStack onUpdate");
		  for (auto layer : m_layerStack)
			layer->onUpdate(timestep);
		}

		m_imGuiLayer->begin();
		{
		  PRIMAL_PROFILE_SCOPE("LayerStack onImGuiRender");
		  for (auto layer : m_layerStack)
			layer->onImGuiRender();
		}
		m_imGuiLayer->end();
	  }

	  m_window->onUpdate();
	}
  }

  bool Application::onWindowClose(WindowCloseEvent& e) {
	m_running = false;
	return true;
  }

  bool Application::onWindowResize(WindowResizeEvent& e) {
	PRIMAL_PROFILE_FUNCTION();
	if (e.getWidth() == 0 || e.getHeight() == 0) {
	  m_minimized = true;
	  return false;
	}

	m_minimized = false;
	Renderer::onWindowResize(e.getWidth(), e.getHeight());

	return false;
  }
}
