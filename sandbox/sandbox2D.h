#pragma once

#include "../src/primal.h"

class Sandbox2D : public primal::Layer {
  public:
	Sandbox2D();
	~Sandbox2D() override = default;

	void onAttach() override;
	void onDetach() override;

	void onUpdate(primal::Timestep ts) override;
	void onEvent(primal::Event& e) override;
	void onImGuiRender() override;

  private:
	primal::OrthographicCameraController m_cameraController;

	// Temp
	primal::ref_ptr<primal::VertexArray> m_squareVA;
	primal::ref_ptr<primal::Shader> m_flatColorShader;

	primal::ref_ptr<primal::Texture2D> m_checkerboardTexture;

	glm::vec4 m_squareColor = { 0.2F, 0.3F, 0.8F, 1.0F };
};
