#include "../extern/imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "primal/core/application.h"
#include "primal/renderer/renderer3D.h"
#include "sandbox3D.h"

Sandbox3D::Sandbox3D() : Layer("Sandbox3D"), m_cameraController(1280.0f / 720.0f) {}

void Sandbox3D::onAttach() {
  PRIMAL_PROFILE_FUNCTION();

  m_modelHouse = primal::createRef<primal::Model>("res/models/cottage/cottage.obj");
  m_modelHouse->loadTexture("res/models/cottage/cottage_diffuse.png", "texture_diffuse");

  m_modelNanosuit = primal::createRef<primal::Model>("res/models/nanosuit/nanosuit.obj");
}

void Sandbox3D::onDetach() {
  PRIMAL_PROFILE_FUNCTION();
}

void Sandbox3D::onUpdate(primal::Timestep ts) {
  PRIMAL_PROFILE_FUNCTION();

  m_lastFrameTime = static_cast<double>(ts.getSeconds());

  m_cameraController.onUpdate(ts);

  {
	PRIMAL_PROFILE_SCOPE("Renderer ShadowMap Render");
	primal::Renderer3D::beginShadowMap();
	primal::Renderer3D::drawModel(m_modelHouse, { 5.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f }, primal::Renderer3D::depthShader());
	primal::Renderer3D::drawModel(m_modelNanosuit, { 1.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f }, primal::Renderer3D::depthShader());
	primal::Renderer3D::endShadowMap();
  }

  {
	PRIMAL_PROFILE_SCOPE("Renderer Offscreen Draw");
	primal::Renderer3D::beginScene(m_cameraController.getCamera());
	primal::Renderer3D::drawModel(m_modelHouse, { 5.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f }, primal::Renderer3D::textureShader());
	primal::Renderer3D::drawModel(m_modelNanosuit, { 1.0f, 0.0f, 0.0f }, { 0.1f, 0.1f, 0.1f }, primal::Renderer3D::textureShader());
	primal::Renderer3D::endScene();
  }

  primal::Renderer3D::debugDepthMap();
}

void Sandbox3D::onImGuiRender() {
  PRIMAL_PROFILE_FUNCTION();

  static bool opt_fullscreen_persistant = true;
  bool opt_fullscreen = opt_fullscreen_persistant;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen) {
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->GetWorkPos());
	ImGui::SetNextWindowSize(viewport->GetWorkSize());
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	window_flags |= ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  bool open = true;
  ImGui::Begin("DockSpace Demo", &open, window_flags);
  ImGui::PopStyleVar();

  if (opt_fullscreen)
	ImGui::PopStyleVar(2);

  // DockSpace
  ImGuiIO& io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  } else {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("ERROR: Docking is not enabled!");
    if (ImGui::SmallButton("Enable Docking"))
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  }

  ImGui::Begin("Debug");

  ImGui::SliderFloat3("DirectionalLight Direction", glm::value_ptr(primal::Renderer3D::lightDirection), -10, 10);
  ImGui::SliderFloat3("PointLight Position", glm::value_ptr(primal::Renderer3D::pointLightPosition), -10, 10);
  ImGui::SliderFloat3("PointLight Position 2", glm::value_ptr(primal::Renderer3D::pointLightPosition2), -10, 10);

  auto cameraPos = m_cameraController.getCamera().getPosition();

  ImGui::Text("Camera Pos: (%f, %f, %f)", static_cast<double>(cameraPos.x), static_cast<double>(cameraPos.y), static_cast<double>(cameraPos.z));
  ImGui::Text("Frame Time: %f s", m_lastFrameTime);

  ImGui::End();

  // NOTE: render scene to panel
  ImGui::Begin("Scene");
  ImGui::Image((void*)(primal::Renderer3D::sceneTextureTarget), ImVec2{ 1280.0f, 720.0f }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
  ImGui::Image((void*)(primal::Renderer3D::depthTextureTarget), ImVec2{ 1024.0f, 1024.0f }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
  ImGui::End();

  if (ImGui::BeginMenuBar()) {
	if (ImGui::BeginMenu("Docking")) {
	  if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_NoSplit;
	  if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_NoResize;
	  if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode;
	  if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode;
	  if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar;
	  ImGui::Separator();
	  ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
  }

  ImGui::End();
}

void Sandbox3D::onEvent(primal::Event& e) {
  m_cameraController.onEvent(e);
}
