#pragma once

#include <utility>

#include "primal/core/input.h"

namespace primal {

  class LinuxInput : public Input {
	protected:
	  bool isKeyPressedImpl(KeyCode keycode) override;

	  bool isMouseButtonPressedImpl(MouseCode button) override;
	  std::pair<float, float> getMousePositionImpl() override;
	  float getMouseXImpl() override;
	  float getMouseYImpl() override;
  };

}
