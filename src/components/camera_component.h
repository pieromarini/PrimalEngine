#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include "fly_camera.h"
#include "scene/component.h"

namespace primal {

  DEFINE_COMPONENT(CameraComponent, Component, true)

  public:
	CameraComponent();
	CameraComponent(const math::vec3& position);

	void onEnable() override;
	void onDisable() override;
	void onDestroy() override;

	void update() override;

	FlyCamera* camera;

  DEFINE_COMPONENT_END(CameraComponent, Component)

}

#endif
