#include <glm/fwd.hpp>
#include <glad/glad.h>

#include "mesh.h"
#include "primal/core/log.h"

namespace primal {

  Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<TextureWrapper> textures) : m_vertices(vertices), m_indices(indices), m_textures(textures) { 
	setupMesh();
  }

  void Mesh::setupMesh() {
	VAO = VertexArray::create();

	VAO->bind();

	VBO = VertexBuffer::create(&m_vertices[0], m_vertices.size() * sizeof(Vertex));

	VBO->setLayout({
	  { ShaderDataType::Float3, "a_Position" },
	  { ShaderDataType::Float3, "a_Normal" },
	  { ShaderDataType::Float2, "a_TexCoords" },
	  { ShaderDataType::Float3, "a_Tangent" },
	  { ShaderDataType::Float3, "a_Bitangent" },
	});

	EBO = IndexBuffer::create(&m_indices[0], m_indices.size());

	VAO->addVertexBuffer(VBO);
	VAO->setIndexBuffer(EBO);

	VAO->unbind();
  }

  void Mesh::draw(Shader* shader) {
	std::size_t diffuseNr = 1;
	std::size_t specularNr = 1;

	// Binding Texture information for Mesh.
	// NOTE: Should this belong here?
	for(std::size_t i = 0; i < m_textures.size(); i++) {
	  glActiveTexture(GL_TEXTURE0 + i);
	  std::string number;
	  std::string name = m_textures[i].type;
	  if(name == "texture_diffuse")
		number = std::to_string(diffuseNr++);
	  else if(name == "texture_specular")
		number = std::to_string(specularNr++);

	  shader->setInt(("u_Material." + name + number).c_str(), i);
	  glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);

	// TODO: Move to Renderer
	VAO->bind();
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
  }


}
