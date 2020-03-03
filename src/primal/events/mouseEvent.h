#pragma once

#include "event.h"
#include "../core/input.h"

namespace primal {

  class MouseMovedEvent : public Event {
	public:
	  MouseMovedEvent(float x, float y) : m_mouseX(x), m_mouseY(y) {}

	  inline float getX() const { return m_mouseX; }
	  inline float getY() const { return m_mouseY; }

	  std::string toString() const override {
		std::stringstream ss;
		ss << "MouseMovedEvent: " << m_mouseX << ", " << m_mouseY;
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(MouseMoved)
	  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
	  float m_mouseX, m_mouseY;
  };

  class MouseScrolledEvent : public Event {
	public:
	  MouseScrolledEvent(float xOffset, float yOffset) : m_xOffset(xOffset), m_yOffset(yOffset) {}

	  inline float getXOffset() const { return m_xOffset; }
	  inline float getYOffset() const { return m_yOffset; }

	  std::string toString() const override {
		std::stringstream ss;
		ss << "MouseScrolledEvent: " << getXOffset() << ", " << getYOffset();
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(MouseScrolled)
	  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	private:
	  float m_xOffset, m_yOffset;
  };

  class MouseButtonEvent : public Event {
	public:
	  inline MouseCode getMouseButton() const { return m_button; }

	  EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
	protected:
	  MouseButtonEvent(MouseCode button) : m_button(button) {}

	  MouseCode m_button;
  };

  class MouseButtonPressedEvent : public MouseButtonEvent {
	public:
	  MouseButtonPressedEvent(MouseCode button) : MouseButtonEvent(button) {}

	  std::string toString() const override {
		std::stringstream ss;
		ss << "MouseButtonPressedEvent: " << m_button;
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(MouseButtonPressed)
  };

  class MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
	  MouseButtonReleasedEvent(MouseCode button) : MouseButtonEvent(button) {}

	  std::string toString() const override {
		std::stringstream ss;
		ss << "MouseButtonReleasedEvent: " << m_button;
		return ss.str();
	  }

	  EVENT_CLASS_TYPE(MouseButtonReleased)
  };

}
