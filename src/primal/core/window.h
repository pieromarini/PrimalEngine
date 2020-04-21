#pragma once

#include <functional>

#include "core.h"
#include "primal/events/event.h"

namespace primal {

  struct WindowProps {
	std::string title;
	unsigned int width;
	unsigned int height;

	WindowProps(const std::string& t = "Primal Engine", unsigned int w = 1280, unsigned int h = 720)
	  : title(t), width(w), height(h) { }
  };

  // Interface for a platform-independent Window representation
  class Window {
	public:
	  using eventCallbackFunc = std::function<void(Event&)>;

	  virtual ~Window() = default;

	  virtual void onUpdate() = 0;

	  virtual unsigned int getWidth() const = 0;
	  virtual unsigned int getHeight() const = 0;

	  // Window attributes
	  virtual void setEventCallback(const eventCallbackFunc& callback) = 0;
	  virtual void setVSync(bool enabled) = 0;
	  virtual bool isVSync() const = 0;

	  virtual void toggleCursor() = 0;
	  virtual int getCursorMode() = 0;

	  virtual void* getNativeWindow() const = 0;

	  static scope_ptr<Window> create(const WindowProps& props = WindowProps());
  };

}
