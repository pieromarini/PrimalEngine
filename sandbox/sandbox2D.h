#pragma once

#include "../primal/primal.h"

class Sandbox2D : public primal::Layer {
  public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void onAttach() override;
	virtual void onDetach() override;

	void onUpdate(primal::Timestep ts) override;
	void onEvent(primal::Event& e) override;
	virtual void onImGuiRender() override;

  private:
	primal::OrthographicCameraController m_cameraController;

	// Temp
	primal::ref_ptr<primal::VertexArray> m_squareVA;
	primal::ref_ptr<primal::Shader> m_flatColorShader;

	primal::ref_ptr<primal::Texture2D> m_checkerboardTexture;

	glm::vec4 m_squareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};
