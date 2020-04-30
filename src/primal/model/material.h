#pragma once

#include <glm/glm.hpp>

#include "primal/core/core.h"
#include "primal/renderer/texture.h"

/*
 * Material.h
 * Materials hold a pointer to each type of texture.
 * They can also hold additional properties (shininess).
 */

namespace primal {

  class Material {
	public:

	  static ref_ptr<Material> create(std::vector<ref_ptr<Texture2D>> textures);

	  std::vector<ref_ptr<Texture2D>> m_textures;
	  float m_shininess{0.5f};
  };

}
