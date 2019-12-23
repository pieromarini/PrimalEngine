#pragma once

#include "event.h"
#include "../core/input.h"

namespace primal {

  class PRIMAL_API KeyEvent : public Event {
	public:
	  inline KeyCode getKeyCode() const { return m_keyCode; }

	  EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
	  KeyEvent(KeyCode keycode) : m_keyCode(keycode) {}

	  KeyCode m_keyCode;
  };

  class PRIMAL_API KeyPressedEvent : public KeyEvent {
	public:
	  KeyPressedEvent(KeyCode keycode, int repeatCount) : KeyEvent(keycode), m_repeatCount(repeatCount) {}

	  inline int getRepeatCount() const { return m_repeatCount; }

	  std::string toString() const override {
		std::stringstream ss;
		ss << "KeyPressedEvent: " << m_keyCode << " (" << m_repeatCount << " repeats)";
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(KeyPressed)
	private:
	  int m_repeatCount;
  };

  class PRIMAL_API KeyReleasedEvent : public KeyEvent {
	public:
	  KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

	  std::string toString() const override {
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << m_keyCode;
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(KeyReleased)
  };

  class PRIMAL_API KeyTypedEvent : public KeyEvent { 
	public:
	  KeyTypedEvent(KeyCode keycode) : KeyEvent(keycode) {}

	  std::string toString() const override {
		std::stringstream ss;
		ss << "KeyTypedEvent: " << m_keyCode;
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(KeyTyped)
  };
}
