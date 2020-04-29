#include "material.h"
#include "primal/renderer/texture.h"

namespace primal {

  Material::Material(const std::string& diffuse) {
	m_textureDiffuse = Texture2D::create(diffuse, "texture_diffuse");
  }

  Material::Material(const std::string& diffuse, const std::string& specular) {
	m_textureDiffuse = Texture2D::create(diffuse, "texture_diffuse");
	m_textureSpecular = Texture2D::create(specular, "texture_specular");
  }

  Material::Material(const std::string& diffuse, const std::string& specular,
					 const std::string& normal) {

	m_textureDiffuse = Texture2D::create(diffuse, "texture_diffuse");
	m_textureSpecular = Texture2D::create(specular, "texture_specular");
	m_textureNormal = Texture2D::create(normal, "texture_normal");
  }

  Material::Material(const std::string& diffuse, const std::string& specular,
					 const std::string& normal, const std::string& height) {

	m_textureDiffuse = Texture2D::create(diffuse, "texture_diffuse");
	m_textureSpecular = Texture2D::create(specular, "texture_specular");
	m_textureNormal = Texture2D::create(normal, "texture_normal");
	m_textureHeight = Texture2D::create(height, "texture_height");
  }
}
