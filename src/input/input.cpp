#include "input.h"

namespace primal {

  InputModule* Input::inputModule{ nullptr };

  void Input::registerWindowCloseCallback(const Action<>& callback) {
	inputModule->registerWindowCloseCallback(callback);
  }

  uint16_t Input::registerWindowSizeCallback(const Action<int, int>& callback) {
	return inputModule->registerWindowSizeCallback(callback);
  }

  void Input::unregisterWindowSizeCallback(uint64_t& handle) {
	inputModule->unregisterWindowSizeCallback(handle);
  }

  bool Input::isKeyPressed(KeyCode key) {
	return inputModule->isKeyPressed(key);
  }

  uint64_t Input::registerKeyPressCallback(KeyCode key, const Action<>& callback) {
	return inputModule->registerKeyPressCallback(key, (ModifierKeys)0, callback);
  }

  uint64_t Input::registerKeyPressCallback(KeyCode key, ModifierKeys mods, const Action<>& callback) {
	return inputModule->registerKeyPressCallback(key, mods, callback);
  }

  void Input::unregisterKeyPressCallback(KeyCode key, uint64_t& handle) {
	inputModule->unregisterKeyPressCallback(key, (ModifierKeys)0, handle);
  }

  void Input::unregisterKeyPressCallback(KeyCode key, ModifierKeys mods, uint64_t& handle) {
	inputModule->unregisterKeyPressCallback(key, mods, handle);
  }

  uint64_t Input::registerKeyReleaseCallback(KeyCode key, const Action<>& callback) {
	return inputModule->registerKeyReleaseCallback(key, (ModifierKeys)0, callback);
  }

  uint64_t Input::registerKeyReleaseCallback(KeyCode key, ModifierKeys mods, const Action<>& callback) {
	return inputModule->registerKeyReleaseCallback(key, mods, callback);
  }

  void Input::unregisterKeyReleaseCallback(KeyCode key, uint64_t& handle) {
	inputModule->unregisterKeyReleaseCallback(key, (ModifierKeys)0, handle);
  }

  void Input::unregisterKeyReleaseCallback(KeyCode key, ModifierKeys mods, uint64_t& handle) {
	inputModule->unregisterKeyReleaseCallback(key, mods, handle);
  }

  bool Input::isMouseButtonPressed(MouseButton mouseButton) {
	return inputModule->isMouseButtonPressed(mouseButton);
  }

  uint64_t Input::registerMousePressCallback(MouseButton mouseButton, const Action<>& callback) {
	return inputModule->registerMousePressCallback(mouseButton, callback);
  }

  void Input::unregisterMousePressCallback(MouseButton mouseButton, uint64_t& handle) {
	inputModule->unregisterMousePressCallback(mouseButton, handle);
  }

  uint64_t Input::registerMouseReleaseCallback(MouseButton mouseButton, const Action<>& callback) {
	return inputModule->registerMouseReleaseCallback(mouseButton, callback);
  }

  void Input::unregisterMouseReleaseCallback(MouseButton mouseButton, uint64_t& handle) {
	inputModule->unregisterMouseReleaseCallback(mouseButton, handle);
  }

  uint16_t Input::registerScrollCallback(const Action<double, double>& callback) {
	return inputModule->registerScrollCallback(callback);
  }

  void Input::unregisterScrollCallback(uint64_t& handle) {
	return inputModule->unregisterScrollCallback(handle);
  }

  float Input::getGamepadAxis(GamepadAxis axis) {
	return inputModule->getGamepadAxis(axis);
  }

  bool Input::isGamepadButtonPressed(GamepadButton button) {
	return inputModule->isGamepadButtonPressed(button);
  }

  void Input::clear() {
	inputModule->clear();
  }

  std::pair<double, double> Input::getMousePosition() {
	return inputModule->getMousePosition();
  }
}
