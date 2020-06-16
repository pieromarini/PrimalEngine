#include <GL/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#include "primal/model/mesh.h"
#include "primal/renderer/buffer.h"
#include "primal/renderer/framebuffer.h"
#include "primal/renderer/orthographicCameraController.h"
#include "renderer3D.h"
#include "renderer2D.h"

#include "primal/core/application.h"
#include "texture.h"
#include "vertexArray.h"
#include "shader.h"
#include "renderCommand.h"


namespace primal {

  glm::vec3 Renderer3D::lightDirection = { 3.0f, 5.0f, 0.0f };
  glm::vec3 Renderer3D::pointLightPosition = { 0.0f, 0.0f, 0.0f };
  glm::vec3 Renderer3D::pointLightPosition2 = { 3.0f, 0.0f, 3.0f };
  uint32_t Renderer3D::sceneTextureTarget{0};
  uint32_t Renderer3D::depthTextureTarget{0};
  static Renderer3DStorage* s_data;

  ref_ptr<Shader> Renderer3D::depthShader() {
	return s_data->depthShader;
  }

  ref_ptr<Shader> Renderer3D::textureShader() {
	return s_data->textureShader;
  }

  void Renderer3D::init() {
	PRIMAL_PROFILE_FUNCTION();

	s_data = new Renderer3DStorage();
	s_data->textureShader = Shader::create("res/shaders/texture.glsl");
	s_data->screenShader = Shader::create("res/shaders/screen_shader.glsl");
	s_data->depthShader = Shader::create("res/shaders/depth_shader.glsl");
	s_data->debugDepthShader = Shader::create("res/shaders/debug_depthmap.glsl");

	s_data->framebuffer = Framebuffer::create(Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight());
	s_data->framebuffer->addTextureAttachment();
	s_data->framebuffer->addDepthStencilAttachment();
	s_data->framebuffer->validate();

	s_data->depthDebugFBO = Framebuffer::create(1024, 1024);
	s_data->depthDebugFBO->addTextureAttachment();
	s_data->depthDebugFBO->addDepthStencilAttachment();
	s_data->depthDebugFBO->validate();

	sceneTextureTarget = s_data->framebuffer->getColorBufferTextureID();
	depthTextureTarget = s_data->depthDebugFBO->getColorBufferTextureID();

	s_data->shadowMapFBO = createRef<ShadowMap>(1024, 1024);

	std::array quadVertices = {
	  -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f
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
	auto ib = IndexBuffer::create(indices.data(), indices.size());
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

	RenderCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1.0f });
	RenderCommand::clear();

	RenderCommand::setDepthTesting(true);

	s_data->textureShader->bind();
	s_data->textureShader->setMat4("u_ViewProjection", camera.getViewProjectionMatrix());
	s_data->textureShader->setFloat3("u_ViewPos", camera.getPosition());
	s_data->textureShader->setMat4("u_LightSpaceMatrix", s_data->lightSpaceMatrix);

	// NOTE: Bind shadow map to texture unit 8
	s_data->textureShader->setInt("u_ShadowMap", 8);
	s_data->shadowMapFBO->bindDepthTexture();

	// Base material data.
	s_data->textureShader->setInt("u_Material.diffuse", 0);
	s_data->textureShader->setInt("u_Material.specular", 1);
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

  void Renderer3D::beginShadowMap() {

	// Shadow map camera view settings
	auto lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 8.5f);
	auto lightView = glm::lookAt(lightDirection, { 0, 0, 0 }, { 0, 1, 0 });
	s_data->lightSpaceMatrix = lightProjection * lightView;

	RenderCommand::clear();
	RenderCommand::setDepthTesting(true);
	s_data->depthShader->bind();
	s_data->depthShader->setMat4("u_LightSpaceMatrix", s_data->lightSpaceMatrix);

	RenderCommand::setViewport(0, 0, 1024, 1024);
	s_data->shadowMapFBO->bind();
	RenderCommand::clearDepth();
  }

  void Renderer3D::endShadowMap() {
	s_data->shadowMapFBO->unbind();
	RenderCommand::setViewport(0, 0, Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight());
	RenderCommand::clear();
  }

  void Renderer3D::debugDepthMap() {
	PRIMAL_PROFILE_FUNCTION();

	RenderCommand::setViewport(0, 0, 1024, 1024);
	s_data->depthDebugFBO->bind();

	RenderCommand::setClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	RenderCommand::clear();

	s_data->debugDepthShader->bind();
	s_data->debugDepthShader->setInt("u_DepthMap", 8);
	s_data->debugDepthShader->setFloat("u_NearPlane", 1.0f);
	s_data->debugDepthShader->setFloat("u_FarPlane", 8.5f);

	s_data->screenQuadVA->bind();
	RenderCommand::setDepthTesting(false);
	s_data->shadowMapFBO->bindDepthTexture();
	RenderCommand::drawIndexed(s_data->screenQuadVA);

	s_data->depthDebugFBO->unbind();
  }

  void Renderer3D::drawScreenQuad() {
	PRIMAL_PROFILE_FUNCTION();

	RenderCommand::setClearColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	RenderCommand::clear();

	s_data->screenShader->bind();
	s_data->screenQuadVA->bind();
	RenderCommand::setDepthTesting(false);
	s_data->framebuffer->bindColorBufferTexture();
	RenderCommand::drawIndexed(s_data->screenQuadVA);
  }

  void Renderer3D::drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size, const ref_ptr<Texture2D>& texture, const ref_ptr<Shader> shader) {
	PRIMAL_PROFILE_FUNCTION();

	texture->bind();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
	shader->setMat4("u_Transform", transform);

	model->draw(shader.get());
  }

  void Renderer3D::drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size, const ref_ptr<Shader> shader) {
	PRIMAL_PROFILE_FUNCTION();

	glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * glm::scale(glm::mat4(1.0f), size);
	shader->setMat4("u_Transform", transform);

	model->draw(shader.get());
  }

}
