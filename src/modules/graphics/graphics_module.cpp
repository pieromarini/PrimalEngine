#include "graphics_module.h"
#include "core/config.h"
#include "components/mesh_component.h"
#include "core/core.h"
#include "resources/resources.h"
#include "scene/entity.h"


namespace primal {

  Unique<GraphicsAPI> GraphicsModule::s_renderingAPI = GraphicsAPI::create(PrimalConfig::renderingAPI);

  void GraphicsModule::init(GLFWwindow* window) {
	m_windowHandle = window; 
	MeshComponent::graphicsModule = this;

	renderer::Resources::init();

	s_renderingAPI->init();

	m_renderer = new renderer::Renderer();
	m_renderer->init();
  }

  void GraphicsModule::update(float deltatime) {
	// NOTE: Submit component data to renderer.
	for (const auto& mesh : meshComponents) {
	  auto isTransformDirty = mesh->entity->getAttribute(Entity::EntityAttributes::IS_TRANSFORM_DIRTY);
	  if (isTransformDirty) {
		mesh->update();
	  }
	}
  }

  void GraphicsModule::shutdown() {
	delete m_renderer;
	renderer::Resources::clean();
  }

  void GraphicsModule::initRenderConfig() {

  }

}
