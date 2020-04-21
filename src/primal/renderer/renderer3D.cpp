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

  void Renderer3D::init() {
	PRIMAL_PROFILE_FUNCTION();

	s_data = new Renderer3DStorage();
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
