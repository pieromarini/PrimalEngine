#include "graphics_module.h"
#include "core/config.h"

namespace primal {

  Unique<GraphicsAPI> GraphicsModule::s_renderingAPI = GraphicsAPI::create(PrimalConfig::renderingAPI);

  void GraphicsModule::init(GLFWwindow* window) {

  }

  void GraphicsModule::update(float deltatime) {
	// TODO: iterate components and render them individually (light, mesh, camera, etc)
  }

  void GraphicsModule::shutdown() {

  }

  void GraphicsModule::initRenderConfig() {

  }

}
