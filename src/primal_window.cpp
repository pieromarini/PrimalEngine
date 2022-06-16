#include "primal_window.h"
#include "application.h"

namespace primal {

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebufferResizedEvent();
}

Window::Window(uint32_t width, uint32_t height, Application* ctx) : m_width{ width }, m_height{ height } {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_handle = glfwCreateWindow(width, height, "Primal Engine", nullptr, nullptr);

	glfwSetWindowUserPointer(m_handle, ctx);
	glfwSetFramebufferSizeCallback(m_handle, framebufferResizeCallback);
}

Window::~Window() {
	glfwDestroyWindow(m_handle);
	glfwTerminate();
}

void Window::init() {
}

bool Window::shouldClose() {
	return glfwWindowShouldClose(m_handle);
}

void Window::processEvents() {
	glfwPollEvents();
}

void Window::createWindowSurface(vk::Instance& instance, VkSurfaceKHR& surface) {
	glfwCreateWindowSurface(instance, m_handle, nullptr, &surface);
}

std::pair<int, int> Window::getFramebufferSize() {
	int width{}, height{};
	glfwGetFramebufferSize(m_handle, &width, &height);
	return { width, height };
}

std::vector<const char*> Window::getGLFWExtensions(const bool enableValidationLayers) {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions{};
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

}// namespace primal
