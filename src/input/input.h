#ifndef INPUT_H
#define INPUT_H

#include "core/data_structures/delegate.h"
#include "key_codes.h"
#include "input_module.h"

namespace primal {

  class Input {
	public:
	  static void registerWindowCloseCallback(const Action<>& callback);
	  static uint16_t registerWindowSizeCallback(const Action<int, int>& callback);
	  static void unregisterWindowSizeCallback(uint64_t& handle);

	  static bool isKeyPressed(KeyCode key);
	  static uint64_t registerKeyPressCallback(KeyCode key, const Action<>& callback);
	  static uint64_t registerKeyPressCallback(KeyCode key, ModifierKeys mods, const Action<>& callback);

	  static void unregisterKeyPressCallback(KeyCode key, uint64_t& handle);
	  static void unregisterKeyPressCallback(KeyCode key, ModifierKeys mods, uint64_t& handle);
	  

	  static uint64_t registerKeyReleaseCallback(KeyCode key, const Action<>& callback);
	  static uint64_t registerKeyReleaseCallback(KeyCode key, ModifierKeys mods, const Action<>& callback);

	  static void unregisterKeyReleaseCallback(KeyCode key, uint64_t& handle);
	  static void unregisterKeyReleaseCallback(KeyCode key, ModifierKeys mods, uint64_t& handle);

	  static std::pair<double, double> getMousePosition();

	  static bool isMouseButtonPressed(MouseButton mouseButton);

	  static uint64_t registerMousePressCallback(MouseButton mouseButton, const Action<>& callback);
	  static void unregisterMousePressCallback(MouseButton mouseButton, uint64_t& handle);

	  static uint64_t registerMouseReleaseCallback(MouseButton mouseButton, const Action<>& callback);
	  static void unregisterMouseReleaseCallback(MouseButton mouseButton, uint64_t& handle);

	  static uint16_t registerScrollCallback(const Action<double, double>& callback);
	  static void unregisterScrollCallback(uint64_t& handle);

	  static float getGamepadAxis(GamepadAxis axis);
	  static bool isGamepadButtonPressed(GamepadButton button);

	  static void clear();

	private:
	  static class InputModule* inputModule;
	  friend class InputModule;
  };

}

#endif
