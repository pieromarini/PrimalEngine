#include "material.h"
#include "primal/renderer/texture.h"

namespace primal {

  // NOTE: My thought here is that when creating a material from a vector of textures,
  // the material instance should get acquire ownership of said vector/textures
  ref_ptr<Material> Material::create(std::vector<ref_ptr<Texture2D>> textures) {
	auto material = createRef<Material>();

	// NOTE: should this move?
	material->m_textures = std::move(textures);

	return material;
  }

}
