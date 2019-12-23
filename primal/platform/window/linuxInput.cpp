#include <GLFW/glfw3.h>

#include "../../primal/core/application.h"
#include "linuxInput.h"

namespace primal {

	Input* Input::s_Instance = new LinuxInput();

	// NOTE: should probably cache the Window.
	bool LinuxInput::isKeyPressedImpl(KeyCode keycode) {
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool LinuxInput::isMouseButtonPressedImpl(MouseCode button) {
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}

	std::pair<float, float> LinuxInput::getMousePositionImpl() {
		auto window = static_cast<GLFWwindow*>(Application::get().getWindow().getNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { static_cast<float>(xpos), static_cast<float>(ypos) };
	}

	float LinuxInput::getMouseXImpl() {
		auto [x, y] = getMousePositionImpl();
		return x;
	}

	float LinuxInput::getMouseYImpl() {
		auto [x, y] = getMousePositionImpl();
		return y;
	}

}
