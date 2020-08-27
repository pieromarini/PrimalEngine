#ifndef RENDER_COMMAND_H
#define RENDER_COMMAND_H

#include "core/math/matrix4.h"
#include "core/math/vector3.h"

namespace primal::renderer {
  class Mesh;
  class Material;

  struct RenderCommand {
	math::Matrix4 transform;
	math::Matrix4 prevTransform;
	Mesh* mesh;
	Material* material;
	math::Vector3 boxMin;
	math::Vector3 boxMax;
  };

}

#endif
