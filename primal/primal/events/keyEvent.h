#pragma once

#include "event.h"

namespace primal {

  class PRIMAL_API KeyEvent : public Event {
	public:
	  inline int getKeyCode() const { return m_KeyCode; }

	  EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
	  KeyEvent(int keycode) : m_KeyCode(keycode) {}

	  int m_KeyCode;
  };

  class PRIMAL_API KeyPressedEvent : public KeyEvent {
	public:
	  KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

	  inline int getRepeatCount() const { return m_RepeatCount; }

	  std::string toString() const override {
		std::stringstream ss;
		ss << "KeyPressedEvent: " << m_KeyCode << " (" << m_RepeatCount << " repeats)";
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(KeyPressed)
	private:
	  int m_RepeatCount;
  };

  class PRIMAL_API KeyReleasedEvent : public KeyEvent {
	public:
	  KeyReleasedEvent(int keycode) : KeyEvent(keycode) {}

	  std::string toString() const override {
		std::stringstream ss;
		ss << "KeyReleasedEvent: " << m_KeyCode;
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(KeyReleased)
  };

  class PRIMAL_API KeyTypedEvent : public KeyEvent { 
	public:
	  KeyTypedEvent(int keycode) : KeyEvent(keycode) {}

	  std::string toString() const override {
		std::stringstream ss;
		ss << "KeyTypedEvent: " << m_KeyCode;
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(KeyTyped)
  };
}
