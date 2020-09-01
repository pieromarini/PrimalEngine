#include "camera_component.h"

#include "tools/log.h"

namespace primal {

  CameraComponent::CameraComponent() {
	camera = new FlyCamera(math::vec3{ 0, 0, 0 });
	PRIMAL_CORE_INFO("Created camera component");
  }

  CameraComponent::CameraComponent(const math::vec3& position) {
	camera = new FlyCamera(position);
	PRIMAL_CORE_INFO("Created camera component");
  }

  void CameraComponent::onEnable() {
	PRIMAL_CORE_INFO("Enabled camera component");
  }

  void CameraComponent::onDisable() {
	PRIMAL_CORE_INFO("Disabled camera component");
  }

  void CameraComponent::onDestroy() {

  }

  void CameraComponent::update() {

  }

}
