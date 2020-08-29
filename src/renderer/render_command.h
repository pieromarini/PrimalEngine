#ifndef RENDER_COMMAND_H
#define RENDER_COMMAND_H

#include "core/math/linear_algebra/matrix.h"
#include "core/math/linear_algebra/vector.h"

namespace primal::renderer {
  class Mesh;
  class Material;

  struct RenderCommand {
	math::mat4 transform;
	math::mat4 prevTransform;
	Mesh* mesh;
	Material* material;
	math::vec3 boxMin;
	math::vec3 boxMax;
  };

}

#endif
