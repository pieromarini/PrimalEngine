#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

#include "core/math/linear_algebra/vector.h"
#include "scene/component.h"
#include "modules/graphics/graphics_module.h"


namespace primal::renderer {

  class Mesh;
  class Material;

}

namespace primal {

  DEFINE_COMPONENT(MeshComponent, Component, true)

  public:
	MeshComponent(renderer::Mesh* mesh, renderer::Material* material, class Transform* transform, math::vec3 boxMin, math::vec3 boxMax);
	MeshComponent(renderer::Mesh* mesh, class Transform* transform, math::vec3 boxMin, math::vec3 boxMax);

	void onEnable() override;
	void onDisable() override;
	void onDestroy() override;

	void update() override;

	friend class GraphicsModule;

	renderer::Mesh* m_mesh;
	renderer::Material* m_material;
	class Transform* m_transform;

	math::vec3 m_boxMin;
	math::vec3 m_boxMax;

  private:
	static class GraphicsModule* graphicsModule;
	

  DEFINE_COMPONENT_END(MeshComponent, Component)

}

#endif
