#ifndef MESH_QUAD_H
#define MESH_QUAD_H

#include "resources/mesh.h"

namespace primal {

  class Quad : public renderer::Mesh {
	public:
	  Quad();
	  Quad(float width, float height);
  };

}

#endif
