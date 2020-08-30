#include <cmath>
#include <GLFW/glfw3.h>

#include "GLFWInput.h"
#include "input_module.h"
#include "input.h"


namespace primal {


  Delegate<> InputModule::windowCloseCallbacks{};

  InputModule::KeyMap InputModule::keyPressDelegates{};
  InputModule::KeyMap InputModule::keyReleaseDelegates{};
  InputModule::MouseMap InputModule::mousePressDelegates{};
  InputModule::MouseMap InputModule::mouseReleaseDelegates{};

  Delegate<int, int> InputModule::windowSizeCallbacks;
  Delegate<double, double> InputModule::scrollCallbacks;

  Delegate<int, int> InputModule::gamepadConnectionCallbacks;

  // GLFW
  Delegate<GLFWwindow*> InputModule::windowCloseGLFWCallbacks;
  Delegate<GLFWwindow*, int, int> InputModule::windowSizeGLFWCallbacks;
  Delegate<GLFWwindow*, int, int, int> InputModule::mouseButtonGLFWCallbacks;
  Delegate<GLFWwindow*, int, int, int, int> InputModule::keyGLFWCallbacks;
  Delegate<GLFWwindow*, double, double> InputModule::scrollGLFWCallbacks;
  Delegate<GLFWwindow*, uint32_t> InputModule::charGLFWCallbacks;

  GLFWwindow* InputModule::m_windowHandle{ nullptr };

  void InputModule::registerWindowCloseCallback(const Action<>& callback) {
	windowCloseCallbacks.subscribe(callback);
  }

  uint64_t InputModule::registerWindowSizeCallback(const Action<int, int>& callback) {
	return windowSizeCallbacks.subscribe(callback);
  }

  void InputModule::unregisterWindowSizeCallback(uint64_t& handle) {
	windowSizeCallbacks.unsubscribe(handle);
  }

  bool InputModule::isKeyPressed(KeyCode key) const {
	auto glfwKey = keyCodeToGLFWKey(key);
	return glfwGetKey(m_windowHandle, glfwKey) == GLFW_PRESS;
  }

  uint64_t InputModule::registerKeyPressCallback(KeyCode key, ModifierKeys mods, const Action<>& callback) {
	return registerCallback(keyCodeToGLFWKey(key), mods, callback, &keyPressDelegates);
  }
  void InputModule::unregisterKeyPressCallback(KeyCode key, ModifierKeys mods, uint64_t& handle) {
	return unregisterCallback(keyCodeToGLFWKey(key), mods, handle, &keyPressDelegates);
  }

  uint64_t InputModule::registerKeyReleaseCallback(KeyCode key, ModifierKeys mods, const Action<>& callback) {
	return registerCallback(keyCodeToGLFWKey(key), mods, callback, &keyReleaseDelegates);
  }
  void InputModule::unregisterKeyReleaseCallback(KeyCode key, ModifierKeys mods, uint64_t& handle) {
	return unregisterCallback(keyCodeToGLFWKey(key), mods, handle, &keyReleaseDelegates);
  }

  std::pair<double, double> InputModule::getMousePosition() const {
	double xPos{};
	double yPos{};
	glfwGetCursorPos(m_windowHandle, &xPos, &yPos);
	return { xPos, yPos };
  }

  bool InputModule::isMouseButtonPressed(MouseButton mouseButton) const {
	auto state = glfwGetMouseButton(m_windowHandle, mouseButtonToGLFWKey(mouseButton));
	return state == GLFW_PRESS;
  }

  uint64_t InputModule::registerMousePressCallback(MouseButton mouseButton, const Action<>& callback) {
	return registerCallback(mouseButtonToGLFWKey(mouseButton), callback, &mousePressDelegates);
  }

  void InputModule::unregisterMousePressCallback(MouseButton mouseButton, uint64_t& handle) {
	return unregisterCallback(mouseButtonToGLFWKey(mouseButton), handle, &mousePressDelegates);
  }

  uint64_t InputModule::registerMouseReleaseCallback(MouseButton mouseButton, const Action<>& callback) {
	return registerCallback(mouseButtonToGLFWKey(mouseButton), callback, &mouseReleaseDelegates);
  }

  void InputModule::unregisterMouseReleaseCallback(MouseButton mouseButton, uint64_t& handle) {
	return unregisterCallback(mouseButtonToGLFWKey(mouseButton), handle, &mouseReleaseDelegates);
  }

  uint64_t InputModule::registerScrollCallback(const Action<double, double>& callback) {
	return scrollCallbacks.subscribe(callback);
  }

  void InputModule::unregisterScrollCallback(uint64_t& handle) {
	scrollCallbacks.unsubscribe(handle);
  }

  void InputModule::registerWindowCloseGLFWCallback(const Action<GLFWwindow*>& callback) {
	windowCloseGLFWCallbacks.subscribe(callback);
  }

  uint64_t InputModule::registerWindowSizeGLFWCallback(const Action<GLFWwindow*, int, int>& callback) {
	return windowSizeGLFWCallbacks.subscribe(callback);
  }

  void InputModule::unregisterWindowSizeGLFWCallback(uint64_t& handle) {
	windowSizeGLFWCallbacks.unsubscribe(handle);
  }

  uint64_t InputModule::registerMouseButtonGLFWCallback(const Action<GLFWwindow*, int, int, int>& callback) {
	return mouseButtonGLFWCallbacks.subscribe(callback);
  }

  void InputModule::unregisterMouseButtonGLFWCallback(uint64_t& handle) {
	mouseButtonGLFWCallbacks.unsubscribe(handle);
  }

  uint64_t InputModule::registerKeyGLFWCallback(const Action<GLFWwindow*, int, int, int, int>& callback) {
	return keyGLFWCallbacks.subscribe(callback);
  }

  void InputModule::unregisterKeyGLFWCallback(uint64_t& handle) {
	keyGLFWCallbacks.unsubscribe(handle);
  }

  uint64_t InputModule::registerScrollGLFWCallback(const Action<GLFWwindow*, double, double>& callback) {
	return scrollGLFWCallbacks.subscribe(callback);
  }

  void InputModule::unregisterScrollGLFWCallback(uint64_t& handle) {
	scrollGLFWCallbacks.unsubscribe(handle);
  }

  uint64_t InputModule::registerCharGLFWCallback(const Action<GLFWwindow*, uint32_t>& callback) {
	return charGLFWCallbacks.subscribe(callback);
  }

  void InputModule::unregisterCharGLFWCallback(uint64_t& handle) {
	charGLFWCallbacks.unsubscribe(handle);
  }

  float InputModule::getGamepadAxis(GamepadAxis axis) {
	return gamepadState.axes[static_cast<int>(axis)];
  }

  bool InputModule::isGamepadButtonPressed(GamepadButton button) {
	return gamepadState.buttons[static_cast<int>(button)];
  }

  void InputModule::init(GLFWwindow* window) {
	m_windowHandle = window;
	Input::inputModule = this;
	GLFWInput::inputModule = this;
	glfwSetInputMode(m_windowHandle, GLFW_STICKY_KEYS, 1);
	glfwSetInputMode(m_windowHandle, GLFW_STICKY_MOUSE_BUTTONS, 1);

	// NOTE: Setup GLFW listeners
	glfwSetWindowCloseCallback(m_windowHandle, windowCloseListener);
	glfwSetWindowSizeCallback(m_windowHandle, windowSizeListener);
	glfwSetKeyCallback(m_windowHandle, keyEventListener);
	glfwSetMouseButtonCallback(m_windowHandle, mouseEventListener);
	glfwSetCharCallback(m_windowHandle, charEventListener);
	glfwSetScrollCallback(m_windowHandle, scrollEventListener);
	glfwSetJoystickCallback(gamepadEventListener);
  }

  void InputModule::update(float deltaTime) {
	glfwPollEvents();
	updateGamepadState();
  }

  void InputModule::shutdown() {}


  uint64_t InputModule::registerCallback(int key, ModifierKeys mods, const Action<>& callback, KeyMap* delegateMap) {
	auto& callbackList = (*delegateMap)[std::make_pair(key, mods)];
	auto handle = callbackList.subscribe(callback);
	return handle;
  }

  void InputModule::unregisterCallback(int key, ModifierKeys mods, uint64_t& handle, KeyMap* delegateMap) {
	auto& callbackList = (*delegateMap)[std::make_pair(key, mods)];
	callbackList.unsubscribe(handle);
  }

  uint64_t InputModule::registerCallback(int key, const Action<>& callback, MouseMap* delegateMap) {
	auto& callbackList = (*delegateMap)[key];
	auto handle = callbackList.subscribe(callback);
	return handle;
  }

  void InputModule::unregisterCallback(int key, uint64_t& handle, MouseMap* delegateMap) {
	auto& callbackList = (*delegateMap)[key];
	callbackList.unsubscribe(handle);
  }

  void InputModule::windowCloseListener(GLFWwindow* window) {
	windowCloseCallbacks.invoke();
	windowCloseGLFWCallbacks.invoke(window);
  }

  void InputModule::keyEventListener(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
	  keyPressDelegates[{ key, static_cast<ModifierKeys>(mods) }].invoke();
	} else if (action == GLFW_RELEASE) {
	  keyReleaseDelegates[{ key, static_cast<ModifierKeys>(mods) }].invoke();
	}

	keyGLFWCallbacks.invoke(window, key, scancode, action, mods);
  }

  void InputModule::mouseEventListener(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
	  mousePressDelegates[button].invoke();
	} else if (action == GLFW_RELEASE) {
	  mousePressDelegates[button].invoke();
	}

	mouseButtonGLFWCallbacks.invoke(window, button, action, mods);
  }

  void InputModule::charEventListener(GLFWwindow* window, uint32_t c) {
	charGLFWCallbacks.invoke(window, c);
  }

  void InputModule::scrollEventListener(GLFWwindow* window, double xoffset, double yoffset) {
	scrollCallbacks.invoke(xoffset, yoffset);
	scrollGLFWCallbacks.invoke(window, xoffset, yoffset);
  }

  void InputModule::gamepadEventListener(int gamepadID, int gamepadEvent) {
	gamepadConnectionCallbacks.invoke(gamepadID, gamepadEvent);
  }

  void InputModule::windowSizeListener(GLFWwindow* window, int width, int height) {
	windowSizeCallbacks.invoke(width, height);
	windowSizeGLFWCallbacks.invoke(window, width, height);
  }

  int InputModule::keyCodeToGLFWKey(KeyCode key) const {
	int glfwKey;
	switch (key) {
	  case KeyCode::NONE:
		glfwKey = GLFW_KEY_UNKNOWN;
		break;
	  case KeyCode::SPACE:
		glfwKey = GLFW_KEY_SPACE;
		break;
	  case KeyCode::APOSTROPHE:
		glfwKey = GLFW_KEY_APOSTROPHE;
		break;
	  case KeyCode::COMMA:
	  case KeyCode::MINUS:
	  case KeyCode::PERIOD:
	  case KeyCode::SLASH:
	  case KeyCode::NUM0:
	  case KeyCode::NUM1:
	  case KeyCode::NUM2:
	  case KeyCode::NUM3:
	  case KeyCode::NUM4:
	  case KeyCode::NUM5:
	  case KeyCode::NUM6:
	  case KeyCode::NUM7:
	  case KeyCode::NUM8:
	  case KeyCode::NUM9:
	  case KeyCode::SEMICOLON:
	  case KeyCode::EQUAL:
		glfwKey = GLFW_KEY_COMMA - static_cast<int>(KeyCode::COMMA) +
		  static_cast<int>(key);
		break;
	  case KeyCode::A:
	  case KeyCode::B:
	  case KeyCode::C:
	  case KeyCode::D:
	  case KeyCode::E:
	  case KeyCode::F:
	  case KeyCode::G:
	  case KeyCode::H:
	  case KeyCode::I:
	  case KeyCode::J:
	  case KeyCode::K:
	  case KeyCode::L:
	  case KeyCode::M:
	  case KeyCode::N:
	  case KeyCode::O:
	  case KeyCode::P:
	  case KeyCode::Q:
	  case KeyCode::R:
	  case KeyCode::S:
	  case KeyCode::T:
	  case KeyCode::U:
	  case KeyCode::V:
	  case KeyCode::W:
	  case KeyCode::X:
	  case KeyCode::Y:
	  case KeyCode::Z:
	  case KeyCode::LEFT_BRACKET:
	  case KeyCode::BACKSLASH:
	  case KeyCode::RIGHT_BRACKET:
		glfwKey =
		  GLFW_KEY_A - static_cast<int>(KeyCode::A) + static_cast<int>(key);
		break;
	  case KeyCode::GRAVE_ACCENT:
		glfwKey = 96;
		break;
	  case KeyCode::ESCAPE:
	  case KeyCode::ENTER:
	  case KeyCode::TAB:
	  case KeyCode::BACKSPACE:
	  case KeyCode::INSERT:
	  case KeyCode::DEL:
	  case KeyCode::RIGHT_ARROW:
	  case KeyCode::LEFT_ARROW:
	  case KeyCode::DOWN_ARROW:
	  case KeyCode::UP_ARROW:
	  case KeyCode::PAGE_UP:
	  case KeyCode::PAGE_DOWN:
	  case KeyCode::HOME:
	  case KeyCode::END:
		glfwKey = GLFW_KEY_ESCAPE - static_cast<int>(KeyCode::ESCAPE) +
		  static_cast<int>(key);
		break;
	  case KeyCode::CAPS_LOCK:
	  case KeyCode::SCROLL_LOCK:
	  case KeyCode::NUM_LOCK:
	  case KeyCode::PRINT_SCREEN:
	  case KeyCode::PAUSE:
		glfwKey = GLFW_KEY_CAPS_LOCK - static_cast<int>(KeyCode::CAPS_LOCK) +
		  static_cast<int>(key);
		break;
	  case KeyCode::F1:
	  case KeyCode::F2:
	  case KeyCode::F3:
	  case KeyCode::F4:
	  case KeyCode::F5:
	  case KeyCode::F6:
	  case KeyCode::F7:
	  case KeyCode::F8:
	  case KeyCode::F9:
	  case KeyCode::F10:
	  case KeyCode::F11:
	  case KeyCode::F12:
	  case KeyCode::F13:
	  case KeyCode::F14:
	  case KeyCode::F15:
	  case KeyCode::F16:
	  case KeyCode::F17:
	  case KeyCode::F18:
	  case KeyCode::F19:
	  case KeyCode::F20:
	  case KeyCode::F21:
	  case KeyCode::F22:
	  case KeyCode::F23:
	  case KeyCode::F24:
	  case KeyCode::F25:
		glfwKey =
		  GLFW_KEY_F1 - static_cast<int>(KeyCode::F1) + static_cast<int>(key);
		break;
	  case KeyCode::KP_0:
	  case KeyCode::KP_1:
	  case KeyCode::KP_2:
	  case KeyCode::KP_3:
	  case KeyCode::KP_4:
	  case KeyCode::KP_5:
	  case KeyCode::KP_6:
	  case KeyCode::KP_7:
	  case KeyCode::KP_8:
	  case KeyCode::KP_9:
	  case KeyCode::KP_DECIMAL:
	  case KeyCode::KP_DIVIDE:
	  case KeyCode::KP_MULTIPLY:
	  case KeyCode::KP_SUBTRACT:
	  case KeyCode::KP_ADD:
	  case KeyCode::KP_ENTER:
	  case KeyCode::KP_EQUAL:
		glfwKey = GLFW_KEY_KP_0 - static_cast<int>(KeyCode::KP_0) +
		  static_cast<int>(key);
		break;
	  case KeyCode::LEFT_SHIFT:
	  case KeyCode::LEFT_CONTROL:
	  case KeyCode::LEFT_ALT:
	  case KeyCode::LEFT_SUPER:
	  case KeyCode::RIGHT_SHIFT:
	  case KeyCode::RIGHT_CONTROL:
	  case KeyCode::RIGHT_ALT:
	  case KeyCode::RIGHT_SUPER:
	  case KeyCode::MENU:
		glfwKey = GLFW_KEY_LEFT_SHIFT - static_cast<int>(KeyCode::LEFT_SHIFT) +
		  static_cast<int>(key);
		break;
	  default:
		glfwKey = GLFW_KEY_UNKNOWN;
		break;
	}
	return glfwKey;
  }

  int InputModule::mouseButtonToGLFWKey(MouseButton mouseButton) const {
	return static_cast<int>(mouseButton);
  }

  void InputModule::updateGamepadState() {
	glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepadState);
	deadzoneOptimize(&gamepadState.axes[0], &gamepadState.axes[1]);
	deadzoneOptimize(&gamepadState.axes[2], &gamepadState.axes[3]);
  }

  void InputModule::deadzoneOptimize(float* horizontal, float* vertical) {
	// TODO
  }

  uint64_t InputModule::registerGamepadConnectionCallback(const Action<int, int>& callback) {
	return gamepadConnectionCallbacks.subscribe(callback);
  }

  void InputModule::unregisterGamepadConnectionCallback(uint64_t& handle) {
	gamepadConnectionCallbacks.unsubscribe(handle);
  }

  void InputModule::clear() {
	windowCloseCallbacks.clear();
	keyPressDelegates.clear();
	keyReleaseDelegates.clear();
	mousePressDelegates.clear();
	mouseReleaseDelegates.clear();
	windowSizeCallbacks.clear();
	scrollCallbacks.clear();
	gamepadConnectionCallbacks.clear();
  }

}
