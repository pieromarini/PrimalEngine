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
	primal::ref_ptr<primal::Model> m_modelHouse;
	primal::ref_ptr<primal::Model> m_modelNanosuit;
	primal::ref_ptr<primal::Model> m_modelTree;
	primal::ref_ptr<primal::Model> m_modelBackpack;

	double m_lastFrameTime{0};
};
