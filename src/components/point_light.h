#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "core/math/vector3.h"

namespace primal {

  class PointLight {
	public:
	  math::Vector3 position = math::Vector3(0.0f);
	  math::Vector3 color = math::Vector3(1.0f);
	  float intensity = 1.0f;
	  float radius = 1.0f;
	  bool visible = true;
	  bool renderMesh = false;
  };

}

#endif
