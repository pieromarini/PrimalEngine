#ifndef CAMERA_H
#define CAMERA_H

#include "camera_frustum.h"
#include "core/math/matrix4.h"

namespace primal {

  enum CAMERA_MOVEMENT {
	CAMERA_FORWARD,
	CAMERA_BACK,
	CAMERA_LEFT,
	CAMERA_RIGHT,
	CAMERA_UP,
	CAMERA_DOWN,
  };

  /* 
	 Basic root camera. Only does relevant camera calculations with manual forced direction 
	 setters. This camera should only be used in code and not respond to user input; the
	 derived cameras are for user/player interaction.
	 */
  // TODO: This should be a component and be attachable to an entity.
  class Camera {
	public:
	  math::Matrix4 Projection;
	  math::Matrix4 View;

	  math::Vector3 Position = math::Vector3(0.0f, 0.0f, 0.0f);
	  math::Vector3 Forward = math::Vector3(0.0f, 0.0f, -1.0f);
	  math::Vector3 Up = math::Vector3(0.0f, 1.0f, 0.0f);
	  math::Vector3 Right = math::Vector3(1.0f, 0.0f, 0.0f);

	  float FOV;
	  float Aspect;
	  float Near;
	  float Far;
	  bool Perspective;

	  CameraFrustum Frustum;

	private:
	public:
	  Camera();
	  Camera(math::Vector3 position, math::Vector3 forward, math::Vector3 up);

	  void update(float dt);

	  void setPerspective(float fov, float aspect, float near, float far);
	  void setOrthographic(float left, float right, float top, float bottom, float near, float far);

	  void updateView();

	  float frustumHeightAtDistance(float distance);
	  float distanceAtFrustumHeight(float frustumHeight);
  };

}

#endif
