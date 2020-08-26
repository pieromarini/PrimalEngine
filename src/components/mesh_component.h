#ifndef MESH_COMPONENT_H
#define MESH_COMPONENT_H

#include "core/math/vector3.h"
#include "scene/component.h"
#include "modules/graphics/graphics_module.h"

namespace primal {

  DEFINE_COMPONENT(MeshComponent, Component, true)

  public:
	MeshComponent(class Mesh* mesh, class Material* material, class Transform* transform, math::Vector3 boxMin, math::Vector3 boxMax);

	void onEnable() override;
	void onDisable() override;
	void onDestroy() override;

	void update() override;

	friend class GraphicsModule;

  private:
	static class GraphicsModule* graphicsModule;

	class Mesh* m_mesh;
	class Material* m_material;
	class Transform* m_transform;
	math::Vector3 m_boxMin;
	math::Vector3 m_boxMax;

  DEFINE_COMPONENT_END(MeshComponent, Component)

}

#endif
