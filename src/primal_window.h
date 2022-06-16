#pragma once

#define GLFW_INCLUDE_VULKAN
#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <cstdint>


namespace primal {

class Application;

class Window {
  public:
	Window(uint32_t width, uint32_t height, Application* ctx);

	~Window();

	void init();

	bool shouldClose();

	void processEvents();

	void createWindowSurface(vk::Instance& instance, VkSurfaceKHR& surface);

	std::pair<int, int> getFramebufferSize();

	std::vector<const char*> getGLFWExtensions(const bool enableValidationLayers);

  private:
	uint32_t m_width, m_height;
	GLFWwindow* m_handle;
};

}// namespace primal
