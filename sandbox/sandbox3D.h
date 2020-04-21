#pragma once

#include "../src/primal.h"

class Sandbox3D : public primal::Layer {
  public:
	Sandbox3D();
	~Sandbox3D() override = default;

	void onAttach() override;
	void onDetach() override;

	void onUpdate(primal::Timestep ts) override;
	void onEvent(primal::Event& e) override;
	void onImGuiRender() override;

  private:
	primal::PerspectiveCameraController m_cameraController;
	primal::ref_ptr<primal::Model> m_modelTest;
	primal::ref_ptr<primal::Texture2D> m_modelTexture;

	glm::vec4 m_squareColor = { 0.2F, 0.3F, 0.8F, 1.0F };
};
