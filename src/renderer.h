#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vulkan/vulkan.hpp>

#include "primal_window.h"
#include "platform/vulkan/context.h"

namespace primal {

class Renderer {
  public:
	Renderer(Window* window) : m_window{ window }, m_context{ new VulkanContext(m_window) } {}

	~Renderer() {
		delete m_context;
		delete m_window;
	}

	void init() {
		if (m_window == nullptr)
			std::runtime_error("Window not initialized.");

		m_context->init();
	}

	void render() {
	}

  private:
	Window* m_window;
	VulkanContext* m_context;
};

}// namespace primal
