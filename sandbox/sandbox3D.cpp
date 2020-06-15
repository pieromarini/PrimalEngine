#include "../extern/imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "primal/core/application.h"
#include "primal/renderer/renderer3D.h"
#include "sandbox3D.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_cameraController(1280.0f / 720.0f) { }

void Sandbox3D::onAttach() {
  PRIMAL_PROFILE_FUNCTION();

  m_modelHouse = primal::createRef<primal::Model>("res/models/cottage/cottage.obj");
  m_modelHouse->loadTexture("res/models/cottage/cottage_diffuse.png", "texture_diffuse");

  m_modelNanosuit = primal::createRef<primal::Model>("res/models/nanosuit/nanosuit.obj");

  // m_modelTree = primal::createRef<primal::Model>("res/models/highpoly-tree/tree.fbx");
}

void Sandbox3D::onDetach() {
  PRIMAL_PROFILE_FUNCTION();
}

void Sandbox3D::onUpdate(primal::Timestep ts) {
  PRIMAL_PROFILE_FUNCTION();

  m_lastFrameTime = static_cast<double>(ts.getSeconds());

  m_cameraController.onUpdate(ts);

  {
	PRIMAL_PROFILE_SCOPE("Renderer Preparation");
	primal::RenderCommand::setClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	primal::RenderCommand::clear();
  }

  {
	PRIMAL_PROFILE_SCOPE("Renderer Draw");
	primal::Renderer3D::beginScene(m_cameraController.getCamera());
	primal::Renderer3D::drawModel(m_modelHouse, { 5.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f });
	primal::Renderer3D::drawModel(m_modelNanosuit, { 1.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f });
	// primal::Renderer3D::drawModel(m_modelTree, { -5.0f, 0.0f, 0.0f }, { 0.05f, 0.05f, 0.05f });
	primal::Renderer3D::endScene();
  }
}

void Sandbox3D::onImGuiRender() {
  PRIMAL_PROFILE_FUNCTION();
  ImGui::Begin("Settings");

  ImGui::SliderFloat3("DirectionalLight Direction", glm::value_ptr(primal::Renderer3D::lightDirection), -10, 10);
  ImGui::SliderFloat3("PointLight Position", glm::value_ptr(primal::Renderer3D::pointLightPosition), -10, 10);
  ImGui::SliderFloat3("PointLight Position 2", glm::value_ptr(primal::Renderer3D::pointLightPosition2), -10, 10);

  auto cameraPos = m_cameraController.getCamera().getPosition();
  ImGui::Text("Camera Pos: (%f, %f, %f)", static_cast<double>(cameraPos.x),
										  static_cast<double>(cameraPos.y),
										  static_cast<double>(cameraPos.z));

  // NOTE: TMP. Move into a "debug panel" inside the engine.
  ImGui::Text("Frame Time: %f s", m_lastFrameTime);
  // ImGui::Text("FPS: %d", static_cast<int>(1 / m_lastFrameTime));

  ImGui::End();
}

void Sandbox3D::onEvent(primal::Event& e) {
  m_cameraController.onEvent(e);
}
