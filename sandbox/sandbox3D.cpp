#include "../vendor/imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "sandbox3D.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_cameraController(1280.0f / 720.0f) { }

void Sandbox3D::onAttach() {
  PRIMAL_PROFILE_FUNCTION();

  m_modelTest = primal::createRef<primal::Model>("res/models/cube.obj");
  m_checkerboardTexture = primal::Texture2D::create("res/textures/checkerboard.png");
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
	primal::Renderer3D::drawCube({ 0.0f, 0.0f, -1.0f }, { 0.5f, 0.75f, 0.5f }, m_squareColor);
	primal::Renderer3D::drawModel(m_modelTest, { 2.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f });
	primal::Renderer3D::drawCube({ 5.0f, 1.0f, -1.0f }, { 0.5f, 0.75f, 0.5f }, m_squareColor);
	// primal::Renderer3D::drawCube({ 0.0f, 0.0f, 1.0f }, { 10.0f, 10.0f , 0.5f}, m_checkerboardTexture);
	primal::Renderer3D::endScene();
  }
}

void Sandbox3D::onImGuiRender() {
  PRIMAL_PROFILE_FUNCTION();
  ImGui::Begin("Settings");
  ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
  ImGui::End();
}

void Sandbox3D::onEvent(primal::Event& e) {
  m_cameraController.onEvent(e);
}
