#ifndef CAMERA_H
#define CAMERA_H

#include "camera_frustum.h"
#include "core/math/math.h"

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
	  math::mat4 Projection;
	  math::mat4 View;

	  math::vec3 Position = math::vec3(0.0f, 0.0f, 0.0f);
	  math::vec3 Forward = math::vec3(0.0f, 0.0f, -1.0f);
	  math::vec3 Up = math::vec3(0.0f, 1.0f, 0.0f);
	  math::vec3 Right = math::vec3(1.0f, 0.0f, 0.0f);

	  float FOV;
	  float Aspect;
	  float Near;
	  float Far;
	  bool Perspective;

	  CameraFrustum Frustum;

	  Camera();
	  Camera(math::vec3 position, math::vec3 forward, math::vec3 up);

	  void update(float dt);

	  void setPerspective(float fov, float aspect, float near, float far);
	  void setOrthographic(float left, float right, float top, float bottom, float near, float far);

	  void updateView();

	  float frustumHeightAtDistance(float distance);
	  float distanceAtFrustumHeight(float frustumHeight);
  };

}

#endif
