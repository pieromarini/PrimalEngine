#include "renderer.h"
#include "renderer2D.h"
#include "renderer3D.h"

namespace primal {

  Renderer::SceneData* Renderer::s_sceneData = new Renderer::SceneData;

  void Renderer::init() {
	RenderCommand::init();
	Renderer2D::init();
	Renderer3D::init();
  }

  void Renderer::shutdown() {
	Renderer2D::shutdown();
	Renderer3D::shutdown();
  }

  void Renderer::onWindowResize(uint32_t width, uint32_t height) {
	RenderCommand::setViewport(0, 0, width, height);
  }

  void Renderer::beginScene(OrthographicCamera& camera) {
	s_sceneData->viewProjectionMatrix = camera.getViewProjectionMatrix();
  }

  void Renderer::endScene() { }

  void Renderer::submit(const ref_ptr<Shader>& shader, const ref_ptr<VertexArray>& vertexArray, const glm::mat4& transform) {
	shader->bind();
	shader->setMat4("u_ViewProjection", s_sceneData->viewProjectionMatrix);
	shader->setMat4("u_Transform", transform);

	vertexArray->bind();
	RenderCommand::drawIndexed(vertexArray);
  }

}
