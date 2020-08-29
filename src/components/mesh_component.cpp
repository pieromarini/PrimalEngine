#include "mesh_component.h"

#include "modules/graphics/graphics_module.h"
#include "tools/log.h"

#include "resources/mesh.h"
#include "scene/transform.h"
#include "renderer/shading/material.h"

namespace primal {

  GraphicsModule* MeshComponent::graphicsModule{ nullptr };

  MeshComponent::MeshComponent(renderer::Mesh* mesh, renderer::Material* material, Transform* transform, math::vec3 boxMin, math::vec3 boxMax): m_mesh{ mesh }, m_material{ material }, m_transform(transform), m_boxMin{ boxMin }, m_boxMax{ boxMax } { }

  MeshComponent::MeshComponent(renderer::Mesh* mesh, Transform* transform, math::vec3 boxMin, math::vec3 boxMax): m_mesh{ mesh }, m_material{ nullptr }, m_transform(transform), m_boxMin{ boxMin }, m_boxMax{ boxMax } { }

  void MeshComponent::onEnable() {
	PRIMAL_CORE_INFO("Created mesh component");
  }

  void MeshComponent::onDisable() {

  }

  void MeshComponent::onDestroy() {
	PRIMAL_CORE_INFO("Destroyed mesh component");
  }

  void MeshComponent::update() {

  }

}
