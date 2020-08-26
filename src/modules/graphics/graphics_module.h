#ifndef GRAPHICSMODULE_H
#define GRAPHICSMODULE_H

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <list>

#include "graphics_api.h"

namespace primal {

  class GraphicsModule {
	public:
	  struct RenderConfig {};

	  GraphicsModule() = default;
	  ~GraphicsModule() = default;

	  void init(GLFWwindow* window);
	  void update(float deltaTime);
	  void shutdown();

	  void initRenderConfig();

	  GLFWwindow* m_windowHandle;

	  std::list<class MeshComponent*> meshComponents;

	  static Unique<GraphicsAPI> s_renderingAPI;
  };

}

#endif
