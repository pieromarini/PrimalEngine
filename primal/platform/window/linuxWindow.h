#pragma once

#include <string>
#include <GLFW/glfw3.h>

#include "../../primal/window.h"
#include "../../primal/renderer/graphicsContext.h"

namespace primal {

  class LinuxWindow : public Window {
	public:
	  LinuxWindow(const WindowProps& props);
	  virtual ~LinuxWindow();

	  void onUpdate() override;

	  inline unsigned int getWidth() const override { return m_Data.width; }
	  inline unsigned int getHeight() const override { return m_Data.height; }

	  // Window attributes
	  inline void setEventCallback(const EventCallbackFn& callback) override { m_Data.eventCallback = callback; }
	  void setVSync(bool enabled) override;
	  bool isVSync() const override;

	  inline virtual void* getNativeWindow() const override { return m_Window; }
	private:
	  virtual void init(const WindowProps& props);
	  virtual void shutdown();
	private:
	  GLFWwindow* m_Window;
	  GraphicsContext* m_Context;

	  struct WindowData {
		std::string title;
		unsigned int width, height;
		bool vSync;

		EventCallbackFn eventCallback;
	  };

	  WindowData m_Data;
  };

}
