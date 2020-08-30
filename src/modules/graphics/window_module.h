#ifndef WINDOWMODULE_H
#define WINDOWMODULE_H

#include <GLFW/glfw3.h>
#include <string>

#include "window.h"

namespace primal {
  
  class WindowModule {
	public:
	  struct WindowConfig {
		int windowWidth{ 1024 };
		int windowHeight{ 768 };
		std::string windowTitle{ "Primal Engine" };
		int windowFullscreen{ 0 };
		int windowShowCursor{ 1 };
	  };

	  WindowModule() = default;
	  ~WindowModule() = default;
	  
	  // TODO: delete copy/move constructors?

	  void init();
	  void update(float deltaTime);
	  void shutdown();

	  void initWindow();
	  void setFullscreen(bool fullscreen);

	  friend class Window;

	  GLFWwindow* m_windowHandle;

	private:
	  int xPos;
	  int yPos;
	  int width;
	  int height;
  };

}

#endif
