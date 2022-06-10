#pragma once

#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <cstdint>

namespace primal {

class Window {
  public:
	Window(uint32_t width, uint32_t height);

	~Window();

	void init();

	bool shouldClose();

	void processEvents();

	std::vector<const char*> getGLFWExtensions(const bool enableValidationLayers);

  private:
	uint32_t m_width, m_height;
	GLFWwindow* m_handle;
};

}// namespace primal
