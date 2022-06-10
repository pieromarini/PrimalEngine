#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>

namespace primal {

class Window {
  public:
	Window(uint32_t width, uint32_t height) : m_width{ width }, m_height{ height } {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_handle = glfwCreateWindow(width, height, "Primal Engine", nullptr, nullptr);
	}

	~Window() {
		glfwDestroyWindow(m_handle);
		glfwTerminate();
	}

	void init() {
	}

	bool shouldClose() {
		return glfwWindowShouldClose(m_handle)
	}

	void processEvents() {
	}

	std::vector<const char*> getGLFWExtensions(const bool enableValidationLayers) {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

  private:
	uint32_t m_width, m_height;
	GLFWwindow* m_handle;
};

}// namespace primal
