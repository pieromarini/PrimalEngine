#include "shader.h"
#include "modules/graphics/graphics_api.h"
#include "renderer/opengl/opengl_shader.h"
#include "tools/log.h"

namespace primal::renderer {

  static Shader* create(std::string name, std::string vsCode, std::string fsCode, std::vector<std::string> defines = {}) {
	auto currentAPI = GraphicsAPI::getAPI();

	switch (currentAPI) {
	  case RenderingAPI::OPENGL:
		return new opengl::OpenGLShader(name, vsCode, fsCode, defines);
	  case RenderingAPI::VULKAN:
	  case RenderingAPI::DIRECTX:
		PRIMAL_CORE_CRITICAL("Vulkan and DirectX are not supported yet.");
	}

	return nullptr;
  }


}
