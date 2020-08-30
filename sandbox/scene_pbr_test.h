#ifndef SCENE_PBR_TEST_H
#define SCENE_PBR_TEST_H

#include "renderer/renderer.h"
#include "shading/material.h"
#include "shading/shader.h"

class ScenePbrTest {
  private:
	primal::renderer::Renderer* m_Renderer;
	primal::Camera* m_Camera;

	primal::renderer::Material m_PBRMaterial;
	primal::renderer::Material m_PBRIrradianceMaterial;
	primal::renderer::Material m_PBRPrefilterMaterial;
	primal::renderer::Material m_PBRBRDFIntegrateMaterial;
	primal::renderer::Shader* m_PbrShader;
	primal::renderer::Shader* m_PbrIrradianceShader;
	primal::renderer::Shader* m_PbrPrefilterShader;
	primal::renderer::Shader* m_PbrBRDFintegrateShader;

  public:
	ScenePbrTest(primal::renderer::Renderer* renderer, primal::Camera* camera);
	~ScenePbrTest();

	void tnit();

	void update(float dt);
	void render();
};

#endif
