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
	  Material(const std::string& diffuse);
	  Material(const std::string& diffuse, const std::string& specular);
	  Material(const std::string& diffuse, const std::string& specular, const std::string& normal);
	  Material(const std::string& diffuse, const std::string& specular, const std::string& normal, const std::string& height);

	private:
	  ref_ptr<Texture2D> m_textureDiffuse;
	  ref_ptr<Texture2D> m_textureSpecular;
	  ref_ptr<Texture2D> m_textureNormal;
	  ref_ptr<Texture2D> m_textureHeight;
	  float m_shininess{0.5f};
  };
}
