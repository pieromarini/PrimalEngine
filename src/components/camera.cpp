#include "camera.h"

namespace primal {

  Camera::Camera() {}

  Camera::Camera(math::vec3 position, math::vec3 forward, math::vec3 up) : Position(position), Forward(forward), Up(up) {
	updateView();
  }

  void Camera::update(float dt) {
	Frustum.update(this);
  }

  void Camera::setPerspective(float fov, float aspect, float near, float far) {
	Perspective = true;
	Projection = math::perspective(fov, aspect, near, far);
	FOV = fov;
	Aspect = aspect;
	Near = near;
	Far = far;
  }

  void Camera::setOrthographic(float left, float right, float top, float bottom, float near, float far) {
	Perspective = false;
	Projection = math::orthographic(left, right, top, bottom, near, far);
	Near = near;
	Far = far;
  }

  void Camera::updateView() {
	View = math::lookAt(Position, Position + Forward, Up);
  }

  float Camera::frustumHeightAtDistance(float distance) {
	if (Perspective) {
	  return 2.0f * distance * tanf(math::deg2Rad(FOV * 0.5));
	} else {
	  return Frustum.Top.D;
	}
  }

  float Camera::distanceAtFrustumHeight(float frustumHeight) {
	if (Perspective) {
	  return frustumHeight * 0.5f / tanf(math::deg2Rad(FOV * 0.5f));
	} else {
	  return Frustum.Near.D;
	}
  }

}
