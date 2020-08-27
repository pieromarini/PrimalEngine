#ifndef PBR_CAPTURE_H
#define PBR_CAPTURE_H

#include "shading/texture_cube.h"
#include "core/math/vector3.h"

namespace primal::renderer {

  struct PBRCapture {
	TextureCube* Irradiance = nullptr;
	TextureCube* Prefiltered = nullptr;

	math::Vector3 Position;
	float Radius;
  };

}

#endif
