#pragma once

#include <vector>
#include <string>

#include "primal/renderer/shader.h"
#include "primal/renderer/texture.h"
#include "primal/renderer/vertexArray.h"
#include "primal/renderer/buffer.h"

namespace primal {

  struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
  };

  class Mesh {
	public:
	  Mesh(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices, std::vector<ref_ptr<Texture2D>> &textures);

	  void draw(Shader* shader);

	  // NOTE: Temporary. just to load textures for a mesh manually.
	  void setTexture(ref_ptr<Texture2D> texture);

	private:
	  void setupMesh();

	  ref_ptr<VertexArray> VAO;
	  ref_ptr<VertexBuffer> VBO;
	  ref_ptr<IndexBuffer> EBO;

	  std::vector<Vertex> m_vertices;
	  std::vector<uint32_t> m_indices;
	  std::vector<ref_ptr<Texture2D>> m_textures;
  };
}
