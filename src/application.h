#pragma once

#include <iostream>
#include <memory>

#include "primal_window.h"
#include "renderer.h"

namespace primal {

class Application {
  public:
	Application() : m_window{ new Window(1920, 1080) }, m_renderer{ new Renderer(m_window) } {}

	~Application() {
		delete m_renderer;
		delete m_window;
	}

	void run() {
		m_window->init();
		m_renderer->init();

		while (!m_window->shouldClose()) {
			m_window->processEvents();

			m_renderer->render();
		}
	}

  private:
	Window* m_window;
	Renderer* m_renderer;
};

}// namespace primal
