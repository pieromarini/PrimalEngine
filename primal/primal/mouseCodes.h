#pragma once

#include <iostream>
#include <stdint.h>

namespace primal {

  // NOTE: Keycodes extraced from glfw3.h
  enum class MouseCode : uint16_t {
	Button0                = 0,
	Button1                = 1,
	Button2                = 2,
	Button3                = 3,
	Button4                = 4,
	Button5                = 5,
	Button6                = 6,
	Button7                = 7,

	ButtonLast             = Button7,
	ButtonLeft             = Button0,
	ButtonRight            = Button1,
	ButtonMiddle           = Button2
  };

  inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode) {
	os << static_cast<int32_t>(mouseCode);
	return os;
  }
}

#define PRIMAL_MOUSE_BUTTON_0      ::primal::MouseCode::Button0
#define PRIMAL_MOUSE_BUTTON_1      ::primal::MouseCode::Button1
#define PRIMAL_MOUSE_BUTTON_2      ::primal::MouseCode::Button2
#define PRIMAL_MOUSE_BUTTON_3      ::primal::MouseCode::Button3
#define PRIMAL_MOUSE_BUTTON_4      ::primal::MouseCode::Button4
#define PRIMAL_MOUSE_BUTTON_5      ::primal::MouseCode::Button5
#define PRIMAL_MOUSE_BUTTON_6      ::primal::MouseCode::Button6
#define PRIMAL_MOUSE_BUTTON_7      ::primal::MouseCode::Button7
#define PRIMAL_MOUSE_BUTTON_LAST   ::primal::MouseCode::ButtonLast
#define PRIMAL_MOUSE_BUTTON_LEFT   ::primal::MouseCode::ButtonLeft
#define PRIMAL_MOUSE_BUTTON_RIGHT  ::primal::MouseCode::ButtonRight
#define PRIMAL_MOUSE_BUTTON_MIDDLE ::primal::MouseCode::ButtonMiddle
