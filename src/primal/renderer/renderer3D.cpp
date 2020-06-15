#include <glm/gtc/matrix_transform.hpp>
#include "primal/model/mesh.h"
#include "primal/renderer/buffer.h"
#include "primal/renderer/framebuffer.h"
#include "renderer3D.h"
#include "renderer2D.h"

#include "primal/core/application.h"
#include "texture.h"
#include "vertexArray.h"
#include "shader.h"
#include "renderCommand.h"


namespace primal {

  struct Renderer3DStorage {
	ref_ptr<Shader> textureShader;
	ref_ptr<Shader> screenShader;
	ref_ptr<Framebuffer> framebuffer;
	ref_ptr<VertexArray> screenQuadVA;
  };

  static Renderer3DStorage* s_data;

  glm::vec3 Renderer3D::lightDirection = { 2.0f, -1.0f, 0.0f };
  glm::vec3 Renderer3D::pointLightPosition = { 0.0f, 0.0f, 0.0f };
  glm::vec3 Renderer3D::pointLightPosition2 = { 3.0f, 0.0f, 3.0f };

  void Renderer3D::init() {
	PRIMAL_PROFILE_FUNCTION();

	s_data = new Renderer3DStorage();
	s_data->textureShader = Shader::create("res/shaders/texture.glsl");
	s_data->screenShader = Shader::create("res/shaders/screen_shader.glsl");

	s_data->framebuffer = Framebuffer::create();
	s_data->framebuffer->addTextureAttachment();
	s_data->framebuffer->addDepthStencilAttachment();
	s_data->framebuffer->validate();

	std::array quadVertices = {
	  -1.0f, -1.0f, 0.0f, 0.0f,
	  1.0f, -1.0f, 1.0f, 0.0f,
	  1.0f,  1.0f, 1.0f, 1.0f,
	  -1.0f,  1.0f, 0.0f, 1.0f
	};
	std::array<uint32_t, 6> indices = {
	  0, 1, 2, 2, 3, 0
	};

	s_data->screenQuadVA = VertexArray::create();
	auto vb = VertexBuffer::create(quadVertices.data(), quadVertices.size() * sizeof(float));
	vb->setLayout({
	  { ShaderDataType::Float2, "a_Position" },
	  { ShaderDataType::Float2, "a_TexCoords" }
	});
	auto ib = IndexBuffer::create(indices.data(), indices.size());;
	s_data->screenQuadVA->addVertexBuffer(vb);
	s_data->screenQuadVA->setIndexBuffer(ib);
  }

  void Renderer3D::shutdown() {
	PRIMAL_PROFILE_FUNCTION();

	delete s_data;
  }

  void Renderer3D::beginScene(const PerspectiveCamera& camera) {
	PRIMAL_PROFILE_FUNCTION();
	s_data->framebuffer->bind();
	RenderCommand::setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
	RenderCommand::clear();

	RenderCommand::setDepthTesting(true);

	s_data->textureShader->bind();
	s_data->textureShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
	s_data->textureShader->setFloat3("u_ViewPos", { 1.0f, 1.0f, 1.0f });

	// Base material data.
	s_data->textureShader->setInt("u_Material.diffuse", 0);
	s_data->textureShader->setInt("u_Material.specular", 0);
	s_data->textureShader->setFloat("u_Material.shininess", 1.0f);

	s_data->textureShader->setFloat3("u_DirLight.direction", lightDirection);
	s_data->textureShader->setFloat3("u_DirLight.ambient", { 0.1f, 0.1f, 0.1f });
	s_data->textureShader->setFloat3("u_DirLight.diffuse", { 0.2f, 0.2f, 0.2f });
	s_data->textureShader->setFloat3("u_DirLight.specular", { 0.1f, 0.1f, 0.1f });

	s_data->textureShader->setInt("u_numPointLights", 2);
	s_data->textureShader->setFloat3("u_PointLights[0].position", pointLightPosition);
	s_data->textureShader->setFloat3("u_PointLights[0].ambient", { 1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_PointLights[0].diffuse", { 1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_PointLights[0].specular", { 0.5f, 0.5f, 0.5f });
	s_data->textureShader->setFloat("u_PointLights[0].constant", 1.0f);
	s_data->textureShader->setFloat("u_PointLights[0].linear", 0.09);
	s_data->textureShader->setFloat("u_PointLights[0].quadratic", 0.032);

	s_data->textureShader->setFloat3("u_PointLights[1].position", pointLightPosition2);
	s_data->textureShader->setFloat3("u_PointLights[1].ambient", { 0.5f, 0.1f, 0.2f });
	s_data->textureShader->setFloat3("u_PointLights[1].diffuse", { 0.5f, 0.1f, 0.2f });
	s_data->textureShader->setFloat3("u_PointLights[1].specular", { 0.8f, 0.8f, 0.8f });
	s_data->textureShader->setFloat("u_PointLights[1].constant", 1.0f);
	s_data->textureShader->setFloat("u_PointLights[1].linear", 0.09);
	s_data->textureShader->setFloat("u_PointLights[1].quadratic", 0.032);

  }

  void Renderer3D::endScene() {
	PRIMAL_PROFILE_FUNCTION();
	s_data->framebuffer->unbind();
	drawScreenQuad();
  }

  // NOTE: Temporary. this renders the framebuffer color texture to a quad.
  void Renderer3D::drawScreenQuad() {
	PRIMAL_PROFILE_FUNCTION();

	RenderCommand::setClearColor({1.0f, 1.0f, 1.0f, 1.0f});
	RenderCommand::clear();

	s_data->screenShader->bind();

	s_data->screenQuadVA->bind();
	RenderCommand::setDepthTesting(false);
	s_data->framebuffer->bindColorBufferTexture();
	RenderCommand::drawIndexed(s_data->screenQuadVA);
  }

  void Renderer3D::drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size, const ref_ptr<Texture2D>& texture) {
	PRIMAL_PROFILE_FUNCTION();

	texture->bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
	s_data->textureShader->setMat4("u_Transform", transform);

	model->draw(s_data->textureShader.get());
  }

  void Renderer3D::drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size) {
	PRIMAL_PROFILE_FUNCTION();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
	s_data->textureShader->setMat4("u_Transform", transform);

	model->draw(s_data->textureShader.get());
  }

}
