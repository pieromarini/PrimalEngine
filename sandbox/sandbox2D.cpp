#include "../extern/imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "sandbox2D.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f) { }

void Sandbox2D::onAttach() {
  PRIMAL_PROFILE_FUNCTION();

  m_checkerboardTexture = primal::Texture2D::create("res/textures/checkerboard.png", "texture_diffuse");
}

void Sandbox2D::onDetach() {
  PRIMAL_PROFILE_FUNCTION();
}

void Sandbox2D::onUpdate(primal::Timestep ts) {
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
	primal::Renderer2D::beginScene(m_cameraController.getCamera());
	primal::Renderer2D::drawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
	primal::Renderer2D::drawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, m_squareColor);
	primal::Renderer2D::drawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_checkerboardTexture);
	primal::Renderer2D::endScene();
  }
}

void Sandbox2D::onImGuiRender() {
  PRIMAL_PROFILE_FUNCTION();
  ImGui::Begin("Settings");
  ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
  ImGui::End();
}

void Sandbox2D::onEvent(primal::Event& e) {
  m_cameraController.onEvent(e);
}
