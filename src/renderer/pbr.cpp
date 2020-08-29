#include "pbr.h"

#include "modules/graphics/graphics_module.h"
#include "renderer.h"
#include "render_target.h"
#include "pbr_capture.h"

#include "resources/primitives/cube.h"
#include "resources/primitives/sphere.h"

#include "resources/resources.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "shading/material.h"
#include "shading/texture.h"
#include "shading/texture_cube.h"
#include "components/camera.h"
#include "components/mesh_component.h"

namespace primal::renderer {

  PBR::PBR(Renderer* renderer) {
	m_renderer = renderer;

	m_renderTargetBRDFLUT = new RenderTarget(128, 128, GL_HALF_FLOAT, 1, true);
	Shader* hdrToCubemap = Resources::loadShader("pbr:hdr_to_cubemap", "shaders/pbr/cube_sample.vs", "shaders/pbr/spherical_to_cube.fs");
	Shader* irradianceCapture = Resources::loadShader("pbr:irradiance", "shaders/pbr/cube_sample.vs", "shaders/pbr/irradiance_capture.fs");
	Shader* prefilterCapture = Resources::loadShader("pbr:prefilter", "shaders/pbr/cube_sample.vs", "shaders/pbr/prefilter_capture.fs");
	Shader* integrateBrdf = Resources::loadShader("pbr:integrate_brdf", "shaders/screen_quad.vs", "shaders/pbr/integrate_brdf.fs");
	m_PBRHdrToCubemap = new Material(hdrToCubemap);
	m_PBRIrradianceCapture = new Material(irradianceCapture);
	m_PBRPrefilterCapture = new Material(prefilterCapture);
	m_PBRIntegrateBRDF = new Material(integrateBrdf);
	m_PBRHdrToCubemap->depthCompare = GL_LEQUAL;
	m_PBRIrradianceCapture->depthCompare = GL_LEQUAL;
	m_PBRPrefilterCapture->depthCompare = GL_LEQUAL;
	m_PBRHdrToCubemap->cull = false;
	m_PBRIrradianceCapture->cull = false;
	m_PBRPrefilterCapture->cull = false;

	m_PBRCaptureCube = new Cube();
	m_sceneEnvCube = primal::Entity::create("Environment Cube", nullptr);
	m_sceneEnvCube->addComponent<primal::MeshComponent>(m_PBRCaptureCube, m_PBRHdrToCubemap, m_sceneEnvCube->transform, -99999, 99999);

	// - brdf integration
	m_renderer->Blit(nullptr, m_renderTargetBRDFLUT, m_PBRIntegrateBRDF);

	// capture
	m_probeCaptureShader = Resources::loadShader("pbr:capture", "shaders/capture.vs", "shaders/capture.fs");
	m_probeCaptureShader->use();
	m_probeCaptureShader->setInt("TexAlbedo", 0);
	m_probeCaptureShader->setInt("TexNormal", 1);
	m_probeCaptureShader->setInt("TexMetallic", 2);
	m_probeCaptureShader->setInt("TexRoughness", 3);
	m_probeCaptureBackgroundShader = Resources::loadShader("pbr:capture background", "shaders/capture_background.vs", "shaders/capture_background.fs");
	m_probeCaptureBackgroundShader->use();
	m_probeCaptureBackgroundShader->setInt("background", 0);

	// debug render
	m_probeDebugSphere = new Sphere(32, 32);
	m_probeDebugShader = Resources::loadShader("pbr:probe_render", "shaders/pbr/probe_render.vs", "shaders/pbr/probe_render.fs");
	m_probeDebugShader->use();
	m_probeDebugShader->setInt("PrefilterMap", 0);
  }

  PBR::~PBR() {
	delete m_PBRCaptureCube;
	delete m_sceneEnvCube;
	delete m_renderTargetBRDFLUT;
	delete m_PBRHdrToCubemap;
	delete m_PBRIrradianceCapture;
	delete m_PBRPrefilterCapture;
	delete m_PBRIntegrateBRDF;
	delete m_skyCapture;
	for (int i = 0; i < m_captureProbes.size(); ++i) {
	  delete m_captureProbes[i]->Irradiance;
	  delete m_captureProbes[i]->Prefiltered;
	  delete m_captureProbes[i];
	}
	delete m_probeDebugSphere;
  }

  void PBR::SetSkyCapture(PBRCapture* capture) {
	m_skyCapture = capture;
  }

  void PBR::AddIrradianceProbe(PBRCapture* capture, math::vec3 position, float radius) {
	capture->Position = position;
	capture->Radius = radius;
	m_captureProbes.push_back(capture);
  }

  void PBR::ClearIrradianceProbes() {
	for (int i = 0; i < m_captureProbes.size(); ++i) {
	  delete m_captureProbes[i]->Irradiance;
	  delete m_captureProbes[i]->Prefiltered;
	  delete m_captureProbes[i];
	}
	m_captureProbes.clear();
  }

  PBRCapture* PBR::ProcessEquirectangular(Texture* envMap) {
	// convert HDR radiance image to HDR environment cubemap
	m_sceneEnvCube->getComponent<MeshComponent>()->m_material = m_PBRHdrToCubemap;
	m_PBRHdrToCubemap->setTexture("environment", envMap, 0);
	TextureCube hdrEnvMap;
	hdrEnvMap.defaultInitialize(128, 128, GL_RGB, GL_FLOAT);
	m_renderer->renderToCubemap(m_sceneEnvCube, &hdrEnvMap);

	return ProcessCube(&hdrEnvMap);
  }

  PBRCapture* PBR::ProcessCube(TextureCube* capture, bool prefilter) {
	PBRCapture* captureProbe = new PBRCapture;

	// irradiance
	captureProbe->Irradiance = new TextureCube;
	captureProbe->Irradiance->defaultInitialize(32, 32, GL_RGB, GL_FLOAT);
	m_PBRIrradianceCapture->setTextureCube("environment", capture, 0);
	m_sceneEnvCube->getComponent<MeshComponent>()->m_material = m_PBRIrradianceCapture;
	m_renderer->renderToCubemap(m_sceneEnvCube, captureProbe->Irradiance, math::vec3(0.0f), 0);
	// prefilter
	if (prefilter) {
	  captureProbe->Prefiltered = new TextureCube;
	  ;
	  captureProbe->Prefiltered->m_filterMin = GL_LINEAR_MIPMAP_LINEAR;
	  captureProbe->Prefiltered->defaultInitialize(128, 128, GL_RGB, GL_FLOAT, true);
	  m_PBRPrefilterCapture->setTextureCube("environment", capture, 0);
	  m_sceneEnvCube->getComponent<MeshComponent>()->m_material = m_PBRPrefilterCapture;
	  // calculate prefilter for multiple roughness levels
	  unsigned int maxMipLevels = 5;
	  for (unsigned int i = 0; i < maxMipLevels; ++i) {
		m_PBRPrefilterCapture->setFloat("roughness", (float)i / (float)(maxMipLevels - 1));
		m_renderer->renderToCubemap(m_sceneEnvCube, captureProbe->Prefiltered, math::vec3(0.0f), i);
	  }
	}
	return captureProbe;
  }

  PBRCapture* PBR::GetSkyCapture() {
	return m_skyCapture;
  }

  std::vector<PBRCapture*> PBR::GetIrradianceProbes(math::vec3 queryPos, float queryRadius) {
	// retrieve all irradiance probes in proximity to queryPos and queryRadius
	std::vector<PBRCapture*> capturesProximity;
	for (int i = 0; i < m_captureProbes.size(); ++i) {
	  float lengthSq = math::lengthSquared((m_captureProbes[i]->Position - queryPos));
	  if (lengthSq < queryRadius * queryRadius) {
		m_captureProbes.push_back(m_captureProbes[i]);
	  }
	}
	// fall back to sky capture if no irradiance probe was found
	if (!capturesProximity.size() == 0) {
	  capturesProximity.push_back(m_skyCapture);
	}
	return capturesProximity;
  }

  void PBR::RenderProbes() {
	m_probeDebugShader->use();
	m_probeDebugShader->setMatrix("projection", m_renderer->GetCamera()->Projection);
	m_probeDebugShader->setMatrix("view", m_renderer->GetCamera()->View);
	m_probeDebugShader->setVector("CamPos", m_renderer->GetCamera()->Position);

	// first render the sky capture
	m_probeDebugShader->setVector("Position", math::vec3(0.0f, 2.0, 0.0f));
	m_skyCapture->Prefiltered->bind(0);
	m_renderer->renderMesh(m_probeDebugSphere, m_probeDebugShader);

	// then do the same for each capture probe (at their respective location)
	for (int i = 0; i < m_captureProbes.size(); ++i) {
	  m_probeDebugShader->setVector("Position", m_captureProbes[i]->Position);
	  if (m_captureProbes[i]->Prefiltered) {
		m_captureProbes[i]->Prefiltered->bind(0);
	  } else {
		m_captureProbes[i]->Irradiance->bind(0);
	  }
	  m_renderer->renderMesh(m_probeDebugSphere, m_probeDebugShader);
	}
  }

}
