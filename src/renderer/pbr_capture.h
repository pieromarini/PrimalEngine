#ifndef PBR_CAPTURE_H
#define PBR_CAPTURE_H

#include "shading/texture_cube.h"
#include "core/math/linear_algebra/vector.h"

namespace primal::renderer {

  struct PBRCapture {
	TextureCube* Irradiance = nullptr;
	TextureCube* Prefiltered = nullptr;

	math::vec3 Position;
	float Radius;
  };

}

#endif
