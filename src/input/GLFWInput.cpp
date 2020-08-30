#include "GLFWInput.h"
#include "input_module.h"

namespace primal {
  InputModule* GLFWInput::inputModule{ nullptr };

  void GLFWInput::registerWindowCloseCallback(const Action<GLFWwindow*>& callback) {
	inputModule->registerWindowCloseGLFWCallback(callback);
  }

  uint64_t GLFWInput::registerWindowSizeCallback(const Action<GLFWwindow*, int, int>& callback) {
	return inputModule->registerWindowSizeGLFWCallback(callback);
  }

  void GLFWInput::unregisterWindowSizeCallback(uint64_t handle) {
	inputModule->unregisterWindowSizeGLFWCallback(handle);
  }

  uint64_t GLFWInput::registerMouseButtonCallback(const Action<GLFWwindow*, int, int, int>& callback) {
	return inputModule->registerMouseButtonGLFWCallback(callback);
  }

  void GLFWInput::unregisterMouseButtonCallback(uint64_t handle) {
	inputModule->unregisterMouseButtonGLFWCallback(handle);
  }

  uint64_t GLFWInput::registerKeyCallback(const Action<GLFWwindow*, int, int, int, int>& callback) {
	return inputModule->registerKeyGLFWCallback(callback);
  }

  void GLFWInput::unregisterKeyCallback(uint64_t handle) {
	inputModule->unregisterKeyGLFWCallback(handle);
  }

  uint64_t GLFWInput::registerScrollCallback(const Action<GLFWwindow*, double, double>& callback) {
	return inputModule->registerScrollGLFWCallback(callback);
  }

  void GLFWInput::unregisterScrollCallback(uint64_t handle) {
	inputModule->unregisterScrollGLFWCallback(handle);
  }

  uint64_t GLFWInput::registerCharCallback(const Action<GLFWwindow*, uint32_t>& callback) {
	return inputModule->registerCharGLFWCallback(callback);
  }

  void GLFWInput::unregisterCharCallback(uint64_t handle) {
	inputModule->unregisterCharGLFWCallback(handle);
  }

}
