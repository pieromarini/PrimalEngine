#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "core/math/linear_algebra/vector.h"

namespace primal {

  class PointLight {
	public:
	  math::vec3 position = math::vec3(0.0f);
	  math::vec3 color = math::vec3(1.0f);
	  float intensity = 1.0f;
	  float radius = 1.0f;
	  bool visible = true;
	  bool renderMesh = false;
  };

}

#endif
