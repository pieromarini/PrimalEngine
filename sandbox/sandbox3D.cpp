#include "../extern/imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "primal/renderer/renderer3D.h"
#include "sandbox3D.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_cameraController(1280.0F / 720.0F) { }

void Sandbox3D::onAttach() {
  PRIMAL_PROFILE_FUNCTION();

  m_modelTest = primal::createRef<primal::Model>("res/models/cottage.obj");
  m_modelTexture = primal::Texture2D::create("res/models/cottage_textures/cottage_diffuse.png");
}

void Sandbox3D::onDetach() {
  PRIMAL_PROFILE_FUNCTION();
}

void Sandbox3D::onUpdate(primal::Timestep ts) {
  PRIMAL_PROFILE_FUNCTION();

  m_cameraController.onUpdate(ts);

  // Render
  {
	PRIMAL_PROFILE_SCOPE("Renderer Preparation");
	primal::RenderCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	primal::RenderCommand::clear();
  }

  {
	PRIMAL_PROFILE_SCOPE("Renderer Draw");
	primal::Renderer3D::beginScene(m_cameraController.getCamera());
	primal::Renderer3D::drawModel(m_modelTest, { 5.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f }, m_modelTexture);
	primal::Renderer3D::endScene();
  }
}

void Sandbox3D::onImGuiRender() {
  PRIMAL_PROFILE_FUNCTION();
  ImGui::Begin("Settings");

  ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
  ImGui::SliderFloat3("Light Direction", glm::value_ptr(primal::Renderer3D::lightDirection), -1, 1);

  auto cameraPos = m_cameraController.getCamera().getPosition();
  ImGui::Text("Camera Pos: (%f, %f, %f)", static_cast<double>(cameraPos.x),
										  static_cast<double>(cameraPos.y),
										  static_cast<double>(cameraPos.z));

  ImGui::End();
}

void Sandbox3D::onEvent(primal::Event& e) {
  m_cameraController.onEvent(e);
}
