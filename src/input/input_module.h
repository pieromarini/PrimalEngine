#ifndef INPUTMODULE_H
#define INPUTMODULE_H

#include <unordered_map>
#include <utility>
#include <GLFW/glfw3.h>

#include "core/data_structures/delegate.h"
#include "util/util.h"
#include "key_codes.h"

namespace primal {

  class InputModule {
	public:
	  void registerWindowCloseCallback(const Action<>& callback);

	  uint64_t registerWindowSizeCallback(const Action<int, int>& callback);
	  void unregisterWindowSizeCallback(uint64_t& handle);

	  [[nodiscard]] bool isKeyPressed(KeyCode key) const;

	  uint64_t registerKeyPressCallback(KeyCode key, ModifierKeys mods, const Action<>& callback);
	  void unregisterKeyPressCallback(KeyCode key, ModifierKeys mods, uint64_t& handle);

	  uint64_t registerKeyReleaseCallback(KeyCode key, ModifierKeys mods, const Action<>& callback);
	  void unregisterKeyReleaseCallback(KeyCode key, ModifierKeys mods, uint64_t& handle);

	  // NOTE: Mouse
	  // TODO: change return value to Vec2 when math library is ready.
	  [[nodiscard]] std::pair<double, double> getMousePosition() const;

	  [[nodiscard]] bool isMouseButtonPressed(MouseButton mouseButton) const;

	  uint64_t registerMousePressCallback(MouseButton mouseButton, const Action<>& callback);
	  void unregisterMousePressCallback(MouseButton mouseButton, uint64_t& handle);

	  uint64_t registerMouseReleaseCallback(MouseButton mouseButton, const Action<>& callback);
	  void unregisterMouseReleaseCallback(MouseButton mouseButton, uint64_t& handle);

	  uint64_t registerScrollCallback(const Action<double, double>& callback);
	  void unregisterScrollCallback(uint64_t& handle);

	  // NOTE: Gamepad
	  float getGamepadAxis(GamepadAxis axis);
	  bool isGamepadButtonPressed(GamepadButton button);

	  uint64_t registerGamepadConnectionCallback(const Action<int, int>& callback);
	  void unregisterGamepadConnectionCallback(uint64_t& handle);

	  void clear();

	  // GLFW Specific callbacks
	  void registerWindowCloseGLFWCallback(const Action<GLFWwindow*>& callback);

	  uint64_t registerWindowSizeGLFWCallback(const Action<GLFWwindow*, int, int>& callback);
	  void unregisterWindowSizeGLFWCallback(uint64_t& handle);

	  uint64_t registerMouseButtonGLFWCallback(const Action<GLFWwindow*, int, int, int>& callback);
	  void unregisterMouseButtonGLFWCallback(uint64_t& handle);

	  uint64_t registerKeyGLFWCallback(const Action<GLFWwindow*, int, int, int, int>& callback);
	  void unregisterKeyGLFWCallback(uint64_t& handle);

	  uint64_t registerScrollGLFWCallback(const Action<GLFWwindow*, double, double>& callback);
	  void unregisterScrollGLFWCallback(uint64_t& handle);

	  uint64_t registerCharGLFWCallback(const Action<GLFWwindow*, uint32_t>& callback);
	  void unregisterCharGLFWCallback(uint64_t& handle);

	private:
	  static GLFWwindow* m_windowHandle;

	  using KeyMap = std::unordered_map<std::pair<int, ModifierKeys>, Delegate<>, util::PairHash, util::PairHash>;
	  using MouseMap = std::unordered_map<int, Delegate<>>;

	  InputModule() = default;
	  ~InputModule() = default;

	  void init(GLFWwindow* window);
	  void update(float deltaTime);
	  void shutdown();

	  uint64_t registerCallback(int key, ModifierKeys mods, const Action<>& callback, KeyMap* delegateMap);
	  void unregisterCallback(int key, ModifierKeys mods, uint64_t& handle, KeyMap* delegateMap);

	  uint64_t registerCallback(int key, const Action<>& callback, MouseMap* delegateMap);
	  void unregisterCallback(int key, uint64_t& handle, MouseMap* delegateMap);

	  [[nodiscard]] int keyCodeToGLFWKey(KeyCode key) const;
	  [[nodiscard]] int mouseButtonToGLFWKey(MouseButton mouseButton) const;

	  GLFWgamepadstate gamepadState;
	  void updateGamepadState();
	  void deadzoneOptimize(float* horizontal, float* vertical);

	  static Delegate<> windowCloseCallbacks;

	  static KeyMap keyPressDelegates;
	  static KeyMap keyReleaseDelegates;
	  static MouseMap mousePressDelegates;
	  static MouseMap mouseReleaseDelegates;

	  static Delegate<int, int> windowSizeCallbacks;
	  static Delegate<double, double> scrollCallbacks;

	  static Delegate<int, int> gamepadConnectionCallbacks;

	  static void windowCloseListener(GLFWwindow* window);
	  static void keyEventListener(GLFWwindow* window, int key, int scancode, int action, int mods);
	  static void mouseEventListener(GLFWwindow* window, int button, int action, int mods);
	  static void charEventListener(GLFWwindow* window, uint32_t c);
	  static void scrollEventListener(GLFWwindow* window, double xoffset, double yoffset);
	  static void gamepadEventListener(int gamepadID, int gamepadEvent);
	  static void windowSizeListener(GLFWwindow* window, int width, int height);

	  // GLFW Callbacks
	  static Delegate<GLFWwindow*> windowCloseGLFWCallbacks;
	  static Delegate<GLFWwindow*, int, int> windowSizeGLFWCallbacks;
	  static Delegate<GLFWwindow*, int, int, int> mouseButtonGLFWCallbacks;
	  static Delegate<GLFWwindow*, int, int, int, int> keyGLFWCallbacks;
	  static Delegate<GLFWwindow*, double, double> scrollGLFWCallbacks;
	  static Delegate<GLFWwindow*, uint32_t> charGLFWCallbacks;

	  friend class EngineLoop;
  };

}

#endif
