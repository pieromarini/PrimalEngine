#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "core/math/linear_algebra/vector.h"
#include "core/math/linear_algebra/matrix.h"

namespace primal::renderer {
  class RenderTarget;
}

namespace primal {

  class DirectionalLight {
	public:
	  math::vec3 direction = math::vec3(0.0f);
	  math::vec3 color = math::vec3(1.0f);
	  float intensity = 1.0f;

	  bool castShadows = true;
	  renderer::RenderTarget* shadowMapRT{ nullptr };
	  math::mat4 lightSpaceViewProjection;
  };

}

#endif
