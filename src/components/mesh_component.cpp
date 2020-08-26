#include "mesh_component.h"

#include "modules/graphics/graphics_module.h"
#include "tools/log.h"

#include "resources/mesh.h"
#include "scene/transform.h"
#include "renderer/shading/material.h"

namespace primal {

  GraphicsModule* MeshComponent::graphicsModule{ nullptr };

  MeshComponent::MeshComponent(Mesh* mesh, Material* material, Transform* transform, math::Vector3 boxMin, math::Vector3 boxMax): m_mesh{ mesh }, m_material{ material }, m_transform(transform), m_boxMin{ boxMin }, m_boxMax{ boxMax } { }

  void MeshComponent::onEnable() {
	PRIMAL_CORE_INFO("Created mesh component");
  }

  void MeshComponent::onDisable() {

  }

  void MeshComponent::onDestroy() {
	PRIMAL_CORE_INFO("Destroyed mesh component");
  }

}
