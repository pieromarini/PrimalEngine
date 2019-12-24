#include "../vendor/imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "sandbox2D.h"

Sandbox2D::Sandbox2D() : Layer("Sandbox2D"), m_cameraController(1280.0f / 720.0f) { }

void Sandbox2D::onAttach() {
  m_checkerboardTexture = primal::Texture2D::create("res/textures/checkerboard.png");
}

void Sandbox2D::onDetach() {

}

void Sandbox2D::onUpdate(primal::Timestep ts) {

  m_cameraController.onUpdate(ts);

  // Render
  primal::RenderCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
  primal::RenderCommand::clear();

  primal::Renderer2D::beginScene(m_cameraController.getCamera());
  primal::Renderer2D::drawQuad({ -1.0f, 0.0f }, { 0.8f, 0.8f }, { 0.8f, 0.2f, 0.3f, 1.0f });
  primal::Renderer2D::drawQuad({ 0.5f, -0.5f }, { 0.5f, 0.75f }, { 0.2f, 0.3f, 0.8f, 1.0f });
  primal::Renderer2D::drawQuad({ 0.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, m_checkerboardTexture);
  primal::Renderer2D::endScene();
}

void Sandbox2D::onImGuiRender() {
  ImGui::Begin("Settings");
  ImGui::ColorEdit4("Square Color", glm::value_ptr(m_squareColor));
  ImGui::End();
}

void Sandbox2D::onEvent(primal::Event& e) {
  m_cameraController.onEvent(e);
}
