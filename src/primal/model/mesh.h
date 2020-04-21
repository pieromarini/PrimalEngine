#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

#include "primal/renderer/shader.h"
#include "primal/renderer/vertexArray.h"
#include "primal/renderer/buffer.h"

namespace primal {

  struct TextureWrapper {
	unsigned int id;
	std::string type;
	std::string path;
  };  

  struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
  };

  class Mesh {
	public:
	  Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<TextureWrapper> textures);

	  void draw(Shader* shader);

	  std::vector<Vertex> m_vertices;
	  std::vector<uint32_t> m_indices;
	  std::vector<TextureWrapper> m_textures;
	private:
	  ref_ptr<VertexArray> VAO;
	  ref_ptr<VertexBuffer> VBO;
	  ref_ptr<IndexBuffer> EBO;
	  void setupMesh();
  };
}
