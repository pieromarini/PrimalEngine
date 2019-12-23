#pragma once

#include <iostream>
#include <stdint.h>

namespace primal {

  // NOTE: Keycodes extraced from glfw3.h
  enum class KeyCode : uint16_t {
	Space               = 32,
	Apostrophe          = 39, /* ' */
	Comma               = 44, /* , */
	Minus               = 45, /* - */
	Period              = 46, /* . */
	Slash               = 47, /* / */

	D0                  = 48, /* 0 */
	D1                  = 49, /* 1 */
	D2                  = 50, /* 2 */
	D3                  = 51, /* 3 */
	D4                  = 52, /* 4 */
	D5                  = 53, /* 5 */
	D6                  = 54, /* 6 */
	D7                  = 55, /* 7 */
	D8                  = 56, /* 8 */
	D9                  = 57, /* 9 */

	Semicolon           = 59, /* ; */
	Equal               = 61, /* = */

	A                   = 65,
	B                   = 66,
	C                   = 67,
	D                   = 68,
	E                   = 69,
	F                   = 70,
	G                   = 71,
	H                   = 72,
	I                   = 73,
	J                   = 74,
	K                   = 75,
	L                   = 76,
	M                   = 77,
	N                   = 78,
	O                   = 79,
	P                   = 80,
	Q                   = 81,
	R                   = 82,
	S                   = 83,
	T                   = 84,
	U                   = 85,
	V                   = 86,
	W                   = 87,
	X                   = 88,
	Y                   = 89,
	Z                   = 90,

	LeftBracket         = 91,  /* [ */
	Backslash           = 92,  /* \ */
	RightBracket        = 93,  /* ] */
	GraveAccent         = 96,  /* ` */

	World1              = 161, /* non-US #1 */
	World2              = 162, /* non-US #2 */

	/* Function keys */
	Escape              = 256,
	Enter               = 257,
	Tab                 = 258,
	Backspace           = 259,
	Insert              = 260,
	Delete              = 261,
	Right               = 262,
	Left                = 263,
	Down                = 264,
	Up                  = 265,
	PageUp              = 266,
	PageDown            = 267,
	Home                = 268,
	End                 = 269,
	CapsLock            = 280,
	ScrollLock          = 281,
	NumLock             = 282,
	PrintScreen         = 283,
	Pause               = 284,
	F1                  = 290,
	F2                  = 291,
	F3                  = 292,
	F4                  = 293,
	F5                  = 294,
	F6                  = 295,
	F7                  = 296,
	F8                  = 297,
	F9                  = 298,
	F10                 = 299,
	F11                 = 300,
	F12                 = 301,
	F13                 = 302,
	F14                 = 303,
	F15                 = 304,
	F16                 = 305,
	F17                 = 306,
	F18                 = 307,
	F19                 = 308,
	F20                 = 309,
	F21                 = 310,
	F22                 = 311,
	F23                 = 312,
	F24                 = 313,
	F25                 = 314,

	/* Keypad */
	KP0                 = 320,
	KP1                 = 321,
	KP2                 = 322,
	KP3                 = 323,
	KP4                 = 324,
	KP5                 = 325,
	KP6                 = 326,
	KP7                 = 327,
	KP8                 = 328,
	KP9                 = 329,
	KPDecimal           = 330,
	KPDivide            = 331,
	KPMultiply          = 332,
	KPSubtract          = 333,
	KPAdd               = 334,
	KPEnter             = 335,
	KPEqual             = 336,

	LeftShift           = 340,
	LeftControl         = 341,
	LeftAlt             = 342,
	LeftSuper           = 343,
	RightShift          = 344,
	RightControl        = 345,
	RightAlt            = 346,
	RightSuper          = 347,
	Menu                = 348
  };

  inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode) {
	os << static_cast<int32_t>(keyCode);
	return os;
  }
}
#define PRIMAL_KEY_SPACE           ::primal::KeyCode::Space
#define PRIMAL_KEY_APOSTROPHE      ::primal::KeyCode::Apostrophe    /* ' */
#define PRIMAL_KEY_COMMA           ::primal::KeyCode::Comma         /* , */
#define PRIMAL_KEY_MINUS           ::primal::KeyCode::Minus         /* - */
#define PRIMAL_KEY_PERIOD          ::primal::KeyCode::Period        /* . */
#define PRIMAL_KEY_SLASH           ::primal::KeyCode::Slash         /* / */
#define PRIMAL_KEY_0               ::primal::KeyCode::D0
#define PRIMAL_KEY_1               ::primal::KeyCode::D1
#define PRIMAL_KEY_2               ::primal::KeyCode::D2
#define PRIMAL_KEY_3               ::primal::KeyCode::D3
#define PRIMAL_KEY_4               ::primal::KeyCode::D4
#define PRIMAL_KEY_5               ::primal::KeyCode::D5
#define PRIMAL_KEY_6               ::primal::KeyCode::D6
#define PRIMAL_KEY_7               ::primal::KeyCode::D7
#define PRIMAL_KEY_8               ::primal::KeyCode::D8
#define PRIMAL_KEY_9               ::primal::KeyCode::D9
#define PRIMAL_KEY_SEMICOLON       ::primal::KeyCode::Semicolon     /* ; */
#define PRIMAL_KEY_EQUAL           ::primal::KeyCode::Equal         /* = */
#define PRIMAL_KEY_A               ::primal::KeyCode::A
#define PRIMAL_KEY_B               ::primal::KeyCode::B
#define PRIMAL_KEY_C               ::primal::KeyCode::C
#define PRIMAL_KEY_D               ::primal::KeyCode::D
#define PRIMAL_KEY_E               ::primal::KeyCode::E
#define PRIMAL_KEY_F               ::primal::KeyCode::F
#define PRIMAL_KEY_G               ::primal::KeyCode::G
#define PRIMAL_KEY_H               ::primal::KeyCode::H
#define PRIMAL_KEY_I               ::primal::KeyCode::I
#define PRIMAL_KEY_J               ::primal::KeyCode::J
#define PRIMAL_KEY_K               ::primal::KeyCode::K
#define PRIMAL_KEY_L               ::primal::KeyCode::L
#define PRIMAL_KEY_M               ::primal::KeyCode::M
#define PRIMAL_KEY_N               ::primal::KeyCode::N
#define PRIMAL_KEY_O               ::primal::KeyCode::O
#define PRIMAL_KEY_P               ::primal::KeyCode::P
#define PRIMAL_KEY_Q               ::primal::KeyCode::Q
#define PRIMAL_KEY_R               ::primal::KeyCode::R
#define PRIMAL_KEY_S               ::primal::KeyCode::S
#define PRIMAL_KEY_T               ::primal::KeyCode::T
#define PRIMAL_KEY_U               ::primal::KeyCode::U
#define PRIMAL_KEY_V               ::primal::KeyCode::V
#define PRIMAL_KEY_W               ::primal::KeyCode::W
#define PRIMAL_KEY_X               ::primal::KeyCode::X
#define PRIMAL_KEY_Y               ::primal::KeyCode::Y
#define PRIMAL_KEY_Z               ::primal::KeyCode::Z
#define PRIMAL_KEY_LEFT_BRACKET    ::primal::KeyCode::LeftBracket   /* [ */
#define PRIMAL_KEY_BACKSLASH       ::primal::KeyCode::Backslash     /* \ */
#define PRIMAL_KEY_RIGHT_BRACKET   ::primal::KeyCode::RightBracket  /* ] */
#define PRIMAL_KEY_GRAVE_ACCENT    ::primal::KeyCode::GraveAccent   /* ` */
#define PRIMAL_KEY_WORLD_1         ::primal::KeyCode::World1        /* non-US #1 */
#define PRIMAL_KEY_WORLD_2         ::primal::KeyCode::World2        /* non-US #2 */

/* Function keys */
#define PRIMAL_KEY_ESCAPE          ::primal::KeyCode::Escape
#define PRIMAL_KEY_ENTER           ::primal::KeyCode::Enter
#define PRIMAL_KEY_TAB             ::primal::KeyCode::Tab
#define PRIMAL_KEY_BACKSPACE       ::primal::KeyCode::Backspace
#define PRIMAL_KEY_INSERT          ::primal::KeyCode::Insert
#define PRIMAL_KEY_DELETE          ::primal::KeyCode::Delete
#define PRIMAL_KEY_RIGHT           ::primal::KeyCode::Right
#define PRIMAL_KEY_LEFT            ::primal::KeyCode::Left
#define PRIMAL_KEY_DOWN            ::primal::KeyCode::Down
#define PRIMAL_KEY_UP              ::primal::KeyCode::Up
#define PRIMAL_KEY_PAGE_UP         ::primal::KeyCode::PageUp
#define PRIMAL_KEY_PAGE_DOWN       ::primal::KeyCode::PageDown
#define PRIMAL_KEY_HOME            ::primal::KeyCode::Home
#define PRIMAL_KEY_END             ::primal::KeyCode::End
#define PRIMAL_KEY_CAPS_LOCK       ::primal::KeyCode::CapsLock
#define PRIMAL_KEY_SCROLL_LOCK     ::primal::KeyCode::ScrollLock
#define PRIMAL_KEY_NUM_LOCK        ::primal::KeyCode::NumLock
#define PRIMAL_KEY_PRINT_SCREEN    ::primal::KeyCode::PrintScreen
#define PRIMAL_KEY_PAUSE           ::primal::KeyCode::Pause
#define PRIMAL_KEY_F1              ::primal::KeyCode::F1
#define PRIMAL_KEY_F2              ::primal::KeyCode::F2
#define PRIMAL_KEY_F3              ::primal::KeyCode::F3
#define PRIMAL_KEY_F4              ::primal::KeyCode::F4
#define PRIMAL_KEY_F5              ::primal::KeyCode::F5
#define PRIMAL_KEY_F6              ::primal::KeyCode::F6
#define PRIMAL_KEY_F7              ::primal::KeyCode::F7
#define PRIMAL_KEY_F8              ::primal::KeyCode::F8
#define PRIMAL_KEY_F9              ::primal::KeyCode::F9
#define PRIMAL_KEY_F10             ::primal::KeyCode::F10
#define PRIMAL_KEY_F11             ::primal::KeyCode::F11
#define PRIMAL_KEY_F12             ::primal::KeyCode::F12
#define PRIMAL_KEY_F13             ::primal::KeyCode::F13
#define PRIMAL_KEY_F14             ::primal::KeyCode::F14
#define PRIMAL_KEY_F15             ::primal::KeyCode::F15
#define PRIMAL_KEY_F16             ::primal::KeyCode::F16
#define PRIMAL_KEY_F17             ::primal::KeyCode::F17
#define PRIMAL_KEY_F18             ::primal::KeyCode::F18
#define PRIMAL_KEY_F19             ::primal::KeyCode::F19
#define PRIMAL_KEY_F20             ::primal::KeyCode::F20
#define PRIMAL_KEY_F21             ::primal::KeyCode::F21
#define PRIMAL_KEY_F22             ::primal::KeyCode::F22
#define PRIMAL_KEY_F23             ::primal::KeyCode::F23
#define PRIMAL_KEY_F24             ::primal::KeyCode::F24
#define PRIMAL_KEY_F25             ::primal::KeyCode::F25

/* Keypad */
#define PRIMAL_KEY_KP_0            ::primal::KeyCode::KP0
#define PRIMAL_KEY_KP_1            ::primal::KeyCode::KP1
#define PRIMAL_KEY_KP_2            ::primal::KeyCode::KP2
#define PRIMAL_KEY_KP_3            ::primal::KeyCode::KP3
#define PRIMAL_KEY_KP_4            ::primal::KeyCode::KP4
#define PRIMAL_KEY_KP_5            ::primal::KeyCode::KP5
#define PRIMAL_KEY_KP_6            ::primal::KeyCode::KP6
#define PRIMAL_KEY_KP_7            ::primal::KeyCode::KP7
#define PRIMAL_KEY_KP_8            ::primal::KeyCode::KP8
#define PRIMAL_KEY_KP_9            ::primal::KeyCode::KP9
#define PRIMAL_KEY_KP_DECIMAL      ::primal::KeyCode::KPDecimal
#define PRIMAL_KEY_KP_DIVIDE       ::primal::KeyCode::KPDivide
#define PRIMAL_KEY_KP_MULTIPLY     ::primal::KeyCode::KPMultiply
#define PRIMAL_KEY_KP_SUBTRACT     ::primal::KeyCode::KPSubtract
#define PRIMAL_KEY_KP_ADD          ::primal::KeyCode::KPAdd
#define PRIMAL_KEY_KP_ENTER        ::primal::KeyCode::KPEnter
#define PRIMAL_KEY_KP_EQUAL        ::primal::KeyCode::KPEqual

#define PRIMAL_KEY_LEFT_SHIFT      ::primal::KeyCode::LeftShift
#define PRIMAL_KEY_LEFT_CONTROL    ::primal::KeyCode::LeftControl
#define PRIMAL_KEY_LEFT_ALT        ::primal::KeyCode::LeftAlt
#define PRIMAL_KEY_LEFT_SUPER      ::primal::KeyCode::LeftSuper
#define PRIMAL_KEY_RIGHT_SHIFT     ::primal::KeyCode::RightShift
#define PRIMAL_KEY_RIGHT_CONTROL   ::primal::KeyCode::RightControl
#define PRIMAL_KEY_RIGHT_ALT       ::primal::KeyCode::RightAlt
#define PRIMAL_KEY_RIGHT_SUPER     ::primal::KeyCode::RightSuper
#define PRIMAL_KEY_MENU            ::primal::KeyCode::Menu
