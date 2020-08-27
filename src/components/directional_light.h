#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "core/math/vector3.h"
#include "core/math/matrix4.h"

namespace primal {

  class RenderTarget;

  class DirectionalLight {
	public:
	  math::Vector3 direction = math::Vector3(0.0f);
	  math::Vector3 color = math::Vector3(1.0f);
	  float intensity = 1.0f;

	  bool castShadows = true;
	  RenderTarget* shadowMapRT{ nullptr };
	  math::Matrix4 lightSpaceViewProjection;
  };

}

#endif
