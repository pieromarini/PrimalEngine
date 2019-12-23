#pragma once

#include <string>
#include <GLFW/glfw3.h>

#include "../../primal/core/window.h"
#include "../../primal/renderer/graphicsContext.h"

namespace primal {

  class LinuxWindow : public Window {
	public:
	  LinuxWindow(const WindowProps& props);
	  virtual ~LinuxWindow();

	  void onUpdate() override;

	  inline unsigned int getWidth() const override { return m_data.width; }
	  inline unsigned int getHeight() const override { return m_data.height; }

	  // Window attributes
	  inline void setEventCallback(const eventCallbackFunc& callback) override { m_data.eventCallback = callback; }
	  void setVSync(bool enabled) override;
	  bool isVSync() const override;

	  inline virtual void* getNativeWindow() const override { return m_window; }
	private:
	  virtual void init(const WindowProps& props);
	  virtual void shutdown();
	private:
	  GLFWwindow* m_window;
	  GraphicsContext* m_context;

	  struct WindowData {
		std::string title;
		unsigned int width, height;
		bool vSync;

		eventCallbackFunc eventCallback;
	  };

	  WindowData m_data;
  };

}
