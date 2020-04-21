#include "renderer2D.h"

#include "primal/core/application.h"
#include "texture.h"
#include "vertexArray.h"
#include "shader.h"
#include "renderCommand.h"

#include <glm/gtc/matrix_transform.hpp>

namespace primal {

  struct Renderer2DStorage {
	ref_ptr<VertexArray> quadVertexArray;
	ref_ptr<Shader> textureShader;
	ref_ptr<Texture2D> whiteTexture;
  };

  static Renderer2DStorage* s_data;

  void Renderer2D::init() {
	PRIMAL_PROFILE_FUNCTION();

	s_data = new Renderer2DStorage();
	s_data->quadVertexArray = VertexArray::create();

	float squareVertices[5 * 4] = {
	  -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
	  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	  0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
	  -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
	};

	auto squareVB = VertexBuffer::create(squareVertices, sizeof(squareVertices));
	squareVB->setLayout({
		{ ShaderDataType::Float3, "a_Position" },
		{ ShaderDataType::Float2, "a_TexCoord" }
		});
	s_data->quadVertexArray->addVertexBuffer(squareVB);

	uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
	auto squareIB = IndexBuffer::create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t));
	s_data->quadVertexArray->setIndexBuffer(squareIB);

	s_data->whiteTexture = Texture2D::create(1, 1);
	uint32_t data = 0xffffffff;
	s_data->whiteTexture->setData(&data, sizeof(uint32_t));

	s_data->textureShader = Shader::create("res/shaders/texture.glsl");
	s_data->textureShader->bind();
	s_data->textureShader->setInt("u_Texture", 0);
  }

  void Renderer2D::shutdown() {
	PRIMAL_PROFILE_FUNCTION();

	delete s_data;
  }

  void Renderer2D::beginScene(const OrthographicCamera& camera) {
	PRIMAL_PROFILE_FUNCTION();

	s_data->textureShader->bind();
	s_data->textureShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
  }

  void Renderer2D::endScene() {
	PRIMAL_PROFILE_FUNCTION();
  }

  void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
	drawQuad({ position.x, position.y, 0.0f }, size, color);
  }

  void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
	PRIMAL_PROFILE_FUNCTION();

	s_data->textureShader->setFloat4("u_Color", color);
	s_data->whiteTexture->bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
	s_data->textureShader->setMat4("u_Transform", transform);
	s_data->quadVertexArray->bind();
	RenderCommand::drawIndexed(s_data->quadVertexArray);
  }

  void Renderer2D::drawQuad(const glm::vec2& position, const glm::vec2& size, const ref_ptr<Texture2D>& texture) {
	drawQuad({ position.x, position.y, 0.0f }, size, texture);
  }

  void Renderer2D::drawQuad(const glm::vec3& position, const glm::vec2& size, const ref_ptr<Texture2D>& texture) {
	PRIMAL_PROFILE_FUNCTION();

	s_data->textureShader->setFloat4("u_Color", glm::vec4(1.0f));
	texture->bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
	s_data->textureShader->setMat4("u_Transform", transform);

	s_data->quadVertexArray->bind();
	RenderCommand::drawIndexed(s_data->quadVertexArray);
  }

}
