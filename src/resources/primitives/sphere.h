#ifndef MESH_SPHERE_H
#define MESH_SPHERE_H

#include "resources/mesh.h"

namespace primal {

  class Sphere : public renderer::Mesh {
	public:
	  Sphere(unsigned int xSegments, unsigned int ySegments);
  };

}

#endif
