#ifndef GLFWINPUT_H
#define GLFWINPUT_H

#include "core/data_structures/delegate.h"
#include "GLFW/glfw3.h"

namespace primal {
  
  class GLFWInput {
	public:
	  static void registerWindowCloseCallback(const Action<GLFWwindow*>& callback);

	  static uint64_t registerWindowSizeCallback(const Action<GLFWwindow*, int, int>& callback);
	  static void unregisterWindowSizeCallback(uint64_t handle);

	  static uint64_t registerMouseButtonCallback(const Action<GLFWwindow*, int, int, int>& callback);
	  static void unregisterMouseButtonCallback(uint64_t handle);
	  
	  static uint64_t registerKeyCallback(const Action<GLFWwindow*, int, int, int, int>& callback);
	  static void unregisterKeyCallback(uint64_t handle);

	  static uint64_t registerScrollCallback(const Action<GLFWwindow*, double, double>& callback);
	  static void unregisterScrollCallback(uint64_t handle);

	  static uint64_t registerCharCallback(const Action<GLFWwindow*, uint32_t>& callback);
	  static void unregisterCharCallback(uint64_t handle);

	private:
	  static class InputModule* inputModule;
	  friend class InputModule;
  };

}

#endif
