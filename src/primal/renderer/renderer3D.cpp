#include "renderer3D.h"

#include "../core/application.h"
#include "texture.h"
#include "vertexArray.h"
#include "shader.h"
#include "renderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace primal {

  struct Renderer3DStorage {
	ref_ptr<VertexArray> cubeVertexArray;
	ref_ptr<Shader> textureShader;
	ref_ptr<Texture2D> whiteTexture;
  };

  static Renderer3DStorage* s_data;

  void Renderer3D::init() {
	PRIMAL_PROFILE_FUNCTION();

	s_data = new Renderer3DStorage();
	s_data->cubeVertexArray = VertexArray::create();

	float cubeVertices[] = {
	  //Back
	  1, 0, 0,
	  0, 0, 0,
	  0, 1, 0,
	  1, 1, 0,

	  //Front
	  0, 0, 1,
	  1, 0, 1,
	  1, 1, 1,
	  0, 1, 1,

	  //Right
	  1, 0, 1,
	  1, 0, 0,
	  1, 1, 0,
	  1, 1, 1,

	  //Left
	  0, 0, 0,
	  0, 0, 1,
	  0, 1, 1,
	  0, 1, 0,

	  //Top
	  0, 1, 1,
	  1, 1, 1,
	  1, 1, 0,
	  0, 1, 0,

	  //Bottom
	  0, 0, 0,
	  1, 0, 0,
	  1, 0, 1,
	  0, 0, 1.
	};

	auto cubeVB = VertexBuffer::create(cubeVertices, sizeof(cubeVertices));
	cubeVB->setLayout({
		{ ShaderDataType::Float3, "a_Position" },
		// { ShaderDataType::Float2, "a_TexCoord" }
	});
	s_data->cubeVertexArray->addVertexBuffer(cubeVB);

	uint32_t cubeIndices[] = {
	  0, 1, 2, 2, 3, 0,
	  4, 5, 6, 6, 7, 4,
	  8, 9, 10, 10, 11, 8,
	  12, 13, 14, 14, 15, 12,
	  16, 17, 18, 18, 19, 16,
	  20, 21, 22, 22, 23, 20
	};
	auto squareIB = IndexBuffer::create(cubeIndices, sizeof(cubeIndices) / sizeof(uint32_t));
	s_data->cubeVertexArray->setIndexBuffer(squareIB);

	s_data->whiteTexture = Texture2D::create(1, 1);
	uint32_t data = 0xffffffff;
	s_data->whiteTexture->setData(&data, sizeof(uint32_t));

	s_data->textureShader = Shader::create("res/shaders/texture.glsl");
	// s_data->textureShader->bind();
	// s_data->textureShader->setInt("u_Texture", 0);
  }

  void Renderer3D::shutdown() {
	PRIMAL_PROFILE_FUNCTION();

	delete s_data;
  }

  void Renderer3D::beginScene(const PerspectiveCamera& camera) {
	PRIMAL_PROFILE_FUNCTION();

	s_data->textureShader->bind();
	s_data->textureShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
  }

  void Renderer3D::endScene() {
	PRIMAL_PROFILE_FUNCTION();
  }

  void Renderer3D::drawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color) {
	PRIMAL_PROFILE_FUNCTION();

	// NOTE: Skipping alpha channel for now
	s_data->textureShader->setFloat3("u_LightColor", {  1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_LightPosition", {  2.0f, 0.5f, 0.0f });
	s_data->whiteTexture->bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
	s_data->textureShader->setMat4("u_Transform", transform);
	s_data->cubeVertexArray->bind();
	RenderCommand::drawIndexed(s_data->cubeVertexArray);
  }

  void Renderer3D::drawCube(const glm::vec3& position, const glm::vec3& size, const ref_ptr<Texture2D>& texture) {
	PRIMAL_PROFILE_FUNCTION();

	s_data->textureShader->setFloat4("u_Color", glm::vec4(1.0f));
	texture->bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
	s_data->textureShader->setMat4("u_Transform", transform);

	s_data->cubeVertexArray->bind();
	RenderCommand::drawIndexed(s_data->cubeVertexArray);
  }

  void Renderer3D::drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size, const ref_ptr<Texture2D>& texture) {
	PRIMAL_PROFILE_FUNCTION();
	s_data->textureShader->setFloat3("u_LightColor", {  1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_LightPosition", {  2.0f, 0.5f, 0.0f });
	texture->bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
	s_data->textureShader->setMat4("u_Transform", transform);

	model->draw(s_data->textureShader.get());
  }

}