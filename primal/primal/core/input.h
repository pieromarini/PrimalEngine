#pragma once

#include <utility>

#include "core.h"
#include "keyCodes.h"
#include "mouseCodes.h"

namespace primal {

  class Input {
	public:
	  inline static bool isKeyPressed(KeyCode keycode) { return s_Instance->isKeyPressedImpl(keycode); }

	  inline static bool isMouseButtonPressed(MouseCode button) { return s_Instance->isMouseButtonPressedImpl(button); }
	  inline static std::pair<float, float> getMousePosition() { return s_Instance->getMousePositionImpl(); }
	  inline static float getMouseX() { return s_Instance->getMouseXImpl(); }
	  inline static float getMouseY() { return s_Instance->getMouseYImpl(); }
	protected:
	  virtual bool isKeyPressedImpl(KeyCode keycode) = 0;

	  virtual bool isMouseButtonPressedImpl(MouseCode button) = 0;
	  virtual std::pair<float, float> getMousePositionImpl() = 0;
	  virtual float getMouseXImpl() = 0;
	  virtual float getMouseYImpl() = 0;
	private:
	  static Input* s_Instance;
  };

}
