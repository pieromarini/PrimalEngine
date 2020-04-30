#include "mesh.h"
#include "primal/core/log.h"
#include "primal/renderer/renderCommand.h"

namespace primal {

  Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<ref_ptr<Texture2D>>& textures) : m_vertices(vertices), m_indices(indices) { 
	m_material = Material::create(textures);
	setupMesh();
  }

  Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, const ref_ptr<Material> material) : m_vertices(vertices), m_indices(indices), m_material(material) { 
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
	std::size_t normalNr = 1;
	std::size_t heightNr = 1;

	// Binding Texture information for Mesh.
	for(std::size_t i = 0; i < m_material->m_textures.size(); ++i) {
	  std::string number;
	  std::string name = m_material->m_textures[i]->m_type;
	  if(name == "texture_diffuse")
		number = std::to_string(diffuseNr++);
	  else if(name == "texture_specular")
		number = std::to_string(specularNr++);
	  else if(name == "texture_normal")
		number = std::to_string(normalNr++);
	  else if(name == "texture_height")
		number = std::to_string(heightNr++);

	  shader->setInt(("u_Material." + name + number).c_str(), i);
	  m_material->m_textures[i]->bind(i);
	}

	RenderCommand::drawIndexed(VAO);
  }

  void Mesh::setTexture(ref_ptr<Texture2D> texture) {
	m_material->m_textures.push_back(texture);
  }


}
