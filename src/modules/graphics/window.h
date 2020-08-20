#ifndef WINDOW_H
#define WINDOW_H

#include <string_view>

namespace primal {

  class Window {
	public:
	  static int getWidth();
	  static int getHeight();

	  static void setTitle(const std::string_view title);

	  // TODO: replace parameters for a "Rect" object
	  static void setSizeLimits(float x, float y, float width, float height);
	  static void setFullscreen(bool fullscreen = true);
	  static bool isFullscreen();

	  enum class CursorMode {
		Normal,
		Hidden,
		Disabled
	  };

	  static void setCursorMode(const CursorMode& mode);

	private:
	  static class WindowModule* s_windowModule;
	  friend class WindowModule;
  };

}

#endif
