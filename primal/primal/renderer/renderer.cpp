#include "renderer.h"

namespace primal {

  Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

  void Renderer::beginScene(OrthographicCamera& camera) {
	s_SceneData->viewProjectionMatrix = camera.getViewProjectionMatrix();
  }

  void Renderer::endScene() { }

  void Renderer::submit(const ref_ptr<Shader>& shader, const ref_ptr<VertexArray>& vertexArray, const glm::mat4& transform) {
	shader->bind();
	shader->uploadUniformMat4("u_ViewProjection", s_SceneData->viewProjectionMatrix);
	shader->uploadUniformMat4("u_Transform", transform);

	vertexArray->bind();
	RenderCommand::drawIndexed(vertexArray);
  }

}
