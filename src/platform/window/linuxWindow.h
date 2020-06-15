#pragma once

#include <string>
#include <GLFW/glfw3.h>

#include "primal/core/window.h"
#include "primal/renderer/graphicsContext.h"

namespace primal {

  class LinuxWindow : public Window {
	public:
	  LinuxWindow(const WindowProps& props);
	   ~LinuxWindow() override;

	  void onUpdate() override;

	  [[nodiscard]] inline unsigned int getWidth() const override { return m_data.width; }
	  [[nodiscard]] inline unsigned int getHeight() const override { return m_data.height; }

	  // Window attributes
	  inline void setEventCallback(const eventCallbackFunc& callback) override { m_data.eventCallback = callback; }
	  void setVSync(bool enabled) override;
	  [[nodiscard]] bool isVSync() const override;

	  void toggleCursor() override;
	  int getCursorMode() override;

	  [[nodiscard]] inline void* getNativeWindow() const override { return m_window; }

	private:
	  void init(const WindowProps& props);
	  void shutdown();

	  GLFWwindow* m_window;
	  scope_ptr<GraphicsContext> m_context;

	  struct WindowData {
		std::string title;
		unsigned int width, height;
		bool vSync;

		eventCallbackFunc eventCallback;
	  };

	  WindowData m_data;
  };

}
