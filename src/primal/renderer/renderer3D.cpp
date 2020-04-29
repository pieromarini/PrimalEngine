#include <glm/gtc/matrix_transform.hpp>
#include "renderer3D.h"

#include "primal/core/application.h"
#include "texture.h"
#include "vertexArray.h"
#include "shader.h"
#include "renderCommand.h"


namespace primal {

  struct Renderer3DStorage {
	ref_ptr<Shader> textureShader;
  };

  static Renderer3DStorage* s_data;

  glm::vec3 Renderer3D::lightDirection = { 2.0f, -1.0f, 0.0f };
  glm::vec3 Renderer3D::pointLightPosition = { 0.0f, 0.0f, 0.0f };

  void Renderer3D::init() {
	PRIMAL_PROFILE_FUNCTION();

	s_data = new Renderer3DStorage();
	s_data->textureShader = Shader::create("res/shaders/texture.glsl");
  }

  void Renderer3D::shutdown() {
	PRIMAL_PROFILE_FUNCTION();

	delete s_data;
  }

  void Renderer3D::beginScene(const PerspectiveCamera& camera) {
	PRIMAL_PROFILE_FUNCTION();

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

	s_data->textureShader->setInt("u_numPointLights", 1);
	s_data->textureShader->setFloat3("u_PointLights[0].position", pointLightPosition);
	s_data->textureShader->setFloat3("u_PointLights[0].ambient", { 1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_PointLights[0].diffuse", { 1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_PointLights[0].specular", { 0.5f, 0.5f, 0.5f });
	s_data->textureShader->setFloat("u_PointLights[0].constant", 1.0f);
	s_data->textureShader->setFloat("u_PointLights[0].linear", 0.09);
	s_data->textureShader->setFloat("u_PointLights[0].quadratic", 0.032);

	/*
	s_data->textureShader->setFloat3("u_SpotLight.position", { 12.0f, 4.5f, 0.0f });
	s_data->textureShader->setFloat3("u_SpotLight.ambient", { 1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_SpotLight.diffuse", { 1.0f, 1.0f, 1.0f });
	s_data->textureShader->setFloat3("u_SpotLight.specular", { 0.5f, 0.5f, 0.5f });
	s_data->textureShader->setFloat("u_SpotLight.constant", 1.0f);
	s_data->textureShader->setFloat("u_SpotLight.linear", 0.09);
	s_data->textureShader->setFloat("u_SpotLight.quadratic", 0.032);
	s_data->textureShader->setFloat("u_SpotLight.cutOff", glm::cos(glm::radians(12.5f)));
	s_data->textureShader->setFloat("u_SpotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
	*/
  }

  void Renderer3D::endScene() {
	PRIMAL_PROFILE_FUNCTION();
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
