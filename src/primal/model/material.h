#pragma once

#include <glm/glm.hpp>

#include "primal/core/core.h"
#include "primal/renderer/texture.h"

/*
 * Material.h
 * Materials hold pointer to a number of textures
 * They can also hold additional material-specific properties (shininess).
 */

namespace primal {

  class Material {
	public:

	  static ref_ptr<Material> create(std::vector<ref_ptr<Texture2D>> textures);

	  std::vector<ref_ptr<Texture2D>> m_textures;
	  // TODO: extract this data from imported models.
	  float m_shininess{0.5f};
  };

}
