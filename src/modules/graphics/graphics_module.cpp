#include "graphics_module.h"
#include "core/config.h"
#include "components/mesh_component.h"
#include "scene/entity.h"

// #include "renderer/renderer.h"

namespace primal {

  Unique<GraphicsAPI> GraphicsModule::s_renderingAPI = GraphicsAPI::create(PrimalConfig::renderingAPI);

  void GraphicsModule::init(GLFWwindow* window) {
	m_windowHandle = window; 
	MeshComponent::graphicsModule = this;
  }

  void GraphicsModule::update(float deltatime) {
	// NOTE: Submit component data to renderer.
	for (const auto& mesh : meshComponents) {
	  auto isTransformDirty = mesh->entity->getAttribute(Entity::EntityAttributes::IS_TRANSFORM_DIRTY);
	  if (isTransformDirty) {
		mesh->update();
		// Renderer::drawMesh(mesh);
	  }
	}
  }

  void GraphicsModule::shutdown() {

  }

  void GraphicsModule::initRenderConfig() {

  }

}
