#include "texture_loader.h"
#include "SOIL/SOIL.h"
#include "SOIL/stb_image_aug.h"
#include "tools/log.h"

namespace primal {

  Texture TextureLoader::loadTexture(std::string path, GLenum m_target, GLenum m_internalFormat, bool srgb) {
	Texture texture;
	texture.m_target = m_target;
	texture.m_internalFormat = m_internalFormat;
	if (texture.m_internalFormat == GL_RGB || texture.m_internalFormat == GL_SRGB)
	  texture.m_internalFormat = srgb ? GL_SRGB : GL_RGB;
	if (texture.m_internalFormat == GL_RGBA || texture.m_internalFormat == GL_SRGB_ALPHA)
	  texture.m_internalFormat = srgb ? GL_SRGB_ALPHA : GL_RGBA;

	int width, height, nrComponents;
	unsigned char* data = SOIL_load_image(path.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
	  GLenum format;
	  if (nrComponents == 1)
		format = GL_RED;
	  else if (nrComponents == 3)
		format = GL_RGB;
	  else if (nrComponents == 4)
		format = GL_RGBA;

	  if (m_target == GL_TEXTURE_1D)
		texture.generate(width, texture.m_internalFormat, format, GL_UNSIGNED_BYTE, data);
	  else if (m_target == GL_TEXTURE_2D)
		texture.generate(width, height, texture.m_internalFormat, format, GL_UNSIGNED_BYTE, data);
	  SOIL_free_image_data(data);
	} else {
	  PRIMAL_CORE_WARN("Texture failed to load at path {0}", path);
	  SOIL_free_image_data(data);
	  return texture;
	}
	texture.m_width = width;
	texture.m_height = height;

	return texture;
  }

  Texture TextureLoader::loadHDRTexture(std::string path) {
	Texture texture;
	texture.m_target = GL_TEXTURE_2D;
	texture.m_filterMin = GL_LINEAR;
	texture.m_mipmapping = false;

	if (stbi_is_hdr(path.c_str())) {
	  int width, height, nrComponents;
	  float* data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
	  if (data) {
		GLenum m_internalFormat, format;
		if (nrComponents == 3) {
		  m_internalFormat = GL_RGB32F;
		  format = GL_RGB;
		} else if (nrComponents == 4) {
		  m_internalFormat = GL_RGBA32F;
		  format = GL_RGBA;
		}

		texture.generate(width, height, m_internalFormat, format, GL_FLOAT, data);
		stbi_image_free(data);
	  }
	  texture.m_width = width;
	  texture.m_height = height;
	} else {
	  PRIMAL_CORE_ERROR("Trying to load an HDR texture with invalid path of texture is not HDR: {0}", path);
	}

	return texture;
  }

  TextureCube TextureLoader::loadTextureCube(std::string top, std::string bottom, std::string left, std::string right, std::string front, std::string back) {
	TextureCube texture;

	std::vector<std::string> faces = { top, bottom, left, right, front, back };
	for (unsigned int i = 0; i < faces.size(); ++i) {
	  int width, height, nrComponents;
	  unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrComponents, 0);

	  if (data) {
		GLenum format;
		if (nrComponents == 3)
		  format = GL_RGB;
		else
		  format = GL_RGBA;

		texture.generateFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, width, height, format, GL_UNSIGNED_BYTE, data);
		SOIL_free_image_data(data);
	  } else {
		PRIMAL_CORE_WARN("Cube texture at path {0} failed to load", faces[i]);
		stbi_image_free(data);
		return texture;
	  }
	}
	if (texture.m_mipmapping)
	  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	return texture;
  }

  TextureCube TextureLoader::loadTextureCube(std::string folder) {
	return TextureLoader::loadTextureCube(folder + "right.jpg",
		folder + "left.jpg",
		folder + "top.jpg",
		folder + "bottom.jpg",
		folder + "front.jpg",
		folder + "back.jpg");
  }

}
