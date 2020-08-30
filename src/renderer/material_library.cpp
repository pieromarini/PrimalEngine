#include "material_library.h"

#include "render_target.h"
#include "renderer/shading/material.h"
#include "resources/resources.h"
#include "tools/log.h"

namespace primal::renderer {

  MaterialLibrary::MaterialLibrary(RenderTarget* gBuffer) {
	generateDefaultMaterials();
	generateInternalMaterials(gBuffer);
  }
  // --------------------------------------------------------------------------------------------
  MaterialLibrary::~MaterialLibrary() {
	for (auto it = m_defaultMaterials.begin(); it != m_defaultMaterials.end(); ++it) {
	  delete it->second;
	}
	for (unsigned int i = 0; i < m_materials.size(); ++i) {
	  delete m_materials[i];
	}
	delete debugLightMaterial;

	delete defaultBlitMaterial;
  }
  // --------------------------------------------------------------------------------------------
  Material* MaterialLibrary::CreateMaterial(std::string base) {
	auto found = m_defaultMaterials.find(SID(base.data()));
	if (found != m_defaultMaterials.end()) {
	  Material copy = *(found->second);
	  Material* mat = new Material(copy);
	  m_materials.push_back(mat);
	  return mat;
	} else {
	  PRIMAL_CORE_ERROR("Material of template: {0} request, but it doesn't exist.", base);
	  return nullptr;
	}
  }
  // --------------------------------------------------------------------------------------------
  Material* MaterialLibrary::CreateCustomMaterial(Shader* shader) {
	Material* mat = new Material(shader);
	mat->type = MATERIAL_CUSTOM;
	m_materials.push_back(mat);
	return mat;
  }
  // --------------------------------------------------------------------------------------------
  Material* MaterialLibrary::CreatePostProcessingMaterial(Shader* shader) {
	Material* mat = new Material(shader);
	mat->type = MATERIAL_POST_PROCESS;
	m_materials.push_back(mat);
	return mat;
  }
  // --------------------------------------------------------------------------------------------
  void MaterialLibrary::generateDefaultMaterials() {
	// default render material (deferred path)
	Shader* defaultShader = Resources::loadShader("default", "shaders/deferred/g_buffer.vs", "shaders/deferred/g_buffer.fs");
	Material* defaultMat = new Material(defaultShader);
	defaultMat->type = MATERIAL_DEFAULT;
	defaultMat->setTexture("TexAlbedo", Resources::loadTexture("default albedo", "textures/checkerboard.png", GL_TEXTURE_2D, GL_RGB), 3);
	defaultMat->setTexture("TexNormal", Resources::loadTexture("default normal", "textures/norm.png"), 4);
	defaultMat->setTexture("TexMetallic", Resources::loadTexture("default metallic", "textures/black.png"), 5);
	defaultMat->setTexture("TexRoughness", Resources::loadTexture("default roughness", "textures/checkerboard.png"), 6);
	m_defaultMaterials[SID("default")] = defaultMat;
	// glass material
	Shader* glassShader = Resources::loadShader("glass", "shaders/forward_render.vs", "shaders/forward_render.fs", { "ALPHA_BLEND" });
	glassShader->use();
	glassShader->setInt("lightShadowMap1", 10);
	glassShader->setInt("lightShadowMap2", 10);
	glassShader->setInt("lightShadowMap3", 10);
	glassShader->setInt("lightShadowMap4", 10);
	Material* glassMat = new Material(glassShader);
	glassMat->type = MATERIAL_CUSTOM;// this material can't fit in the deferred rendering pipeline (due to transparency sorting).
	glassMat->setTexture("TexAlbedo", Resources::loadTexture("glass albedo", "textures/glass.png", GL_TEXTURE_2D, GL_RGBA), 0);
	glassMat->setTexture("TexNormal", Resources::loadTexture("glass normal", "textures/pbr/plastic/normal.png"), 1);
	glassMat->setTexture("TexMetallic", Resources::loadTexture("glass metallic", "textures/pbr/plastic/metallic.png"), 2);
	glassMat->setTexture("TexRoughness", Resources::loadTexture("glass roughness", "textures/pbr/plastic/roughness.png"), 3);
	glassMat->setTexture("TexAO", Resources::loadTexture("glass ao", "textures/pbr/plastic/ao.png"), 4);
	glassMat->blend = true;
	m_defaultMaterials[SID("glass")] = glassMat;
	// alpha blend material
	Shader* alphaBlendShader = Resources::loadShader("alpha blend", "shaders/forward_render.vs", "shaders/forward_render.fs", { "ALPHA_BLEND" });
	alphaBlendShader->use();
	alphaBlendShader->setInt("lightShadowMap1", 10);
	alphaBlendShader->setInt("lightShadowMap2", 10);
	alphaBlendShader->setInt("lightShadowMap3", 10);
	alphaBlendShader->setInt("lightShadowMap4", 10);
	Material* alphaBlendMaterial = new Material(alphaBlendShader);
	alphaBlendMaterial->type = MATERIAL_CUSTOM;
	alphaBlendMaterial->blend = true;
	m_defaultMaterials[SID("alpha blend")] = alphaBlendMaterial;
	// alpha cutout material
	Shader* alphaDiscardShader = Resources::loadShader("alpha discard", "shaders/forward_render.vs", "shaders/forward_render.fs", { "ALPHA_DISCARD" });
	alphaDiscardShader->use();
	alphaDiscardShader->setInt("lightShadowMap1", 10);
	alphaDiscardShader->setInt("lightShadowMap2", 10);
	alphaDiscardShader->setInt("lightShadowMap3", 10);
	alphaDiscardShader->setInt("lightShadowMap4", 10);
	Material* alphaDiscardMaterial = new Material(alphaDiscardShader);
	alphaDiscardMaterial->type = MATERIAL_CUSTOM;
	alphaDiscardMaterial->cull = false;
	m_defaultMaterials[SID("alpha discard")] = alphaDiscardMaterial;
  }
  // --------------------------------------------------------------------------------------------
  void MaterialLibrary::generateInternalMaterials(RenderTarget* gBuffer) {
	// post-processing
	Shader* defaultBlitShader = Resources::loadShader("blit", "shaders/screen_quad.vs", "shaders/default_blit.fs");
	defaultBlitMaterial = new Material(defaultBlitShader);

	// deferred
	deferredAmbientShader = Resources::loadShader("deferred ambient", "shaders/deferred/screen_ambient.vs", "shaders/deferred/ambient.fs");
	deferredIrradianceShader = Resources::loadShader("deferred irradiance", "shaders/deferred/ambient_irradience.vs", "shaders/deferred/ambient_irradience.fs");
	deferredDirectionalShader = Resources::loadShader("deferred directional", "shaders/deferred/screen_directional.vs", "shaders/deferred/directional.fs");
	deferredPointShader = Resources::loadShader("deferred point", "shaders/deferred/point.vs", "shaders/deferred/point.fs");

	deferredAmbientShader->use();
	deferredAmbientShader->setInt("gPositionMetallic", 0);
	deferredAmbientShader->setInt("gNormalRoughness", 1);
	deferredAmbientShader->setInt("gAlbedoAO", 2);
	deferredAmbientShader->setInt("envIrradiance", 3);
	deferredAmbientShader->setInt("envPrefilter", 4);
	deferredAmbientShader->setInt("BRDFLUT", 5);
	deferredAmbientShader->setInt("TexSSAO", 6);
	deferredIrradianceShader->use();
	deferredIrradianceShader->setInt("gPositionMetallic", 0);
	deferredIrradianceShader->setInt("gNormalRoughness", 1);
	deferredIrradianceShader->setInt("gAlbedoAO", 2);
	deferredIrradianceShader->setInt("envIrradiance", 3);
	deferredIrradianceShader->setInt("envPrefilter", 4);
	deferredIrradianceShader->setInt("BRDFLUT", 5);
	deferredIrradianceShader->setInt("TexSSAO", 6);
	deferredDirectionalShader->use();
	deferredDirectionalShader->setInt("gPositionMetallic", 0);
	deferredDirectionalShader->setInt("gNormalRoughness", 1);
	deferredDirectionalShader->setInt("gAlbedoAO", 2);
	deferredDirectionalShader->setInt("lightShadowMap", 3);
	deferredPointShader->use();
	deferredPointShader->setInt("gPositionMetallic", 0);
	deferredPointShader->setInt("gNormalRoughness", 1);
	deferredPointShader->setInt("gAlbedoAO", 2);

	// shadows
	dirShadowShader = Resources::loadShader("shadow directional", "shaders/shadow_cast.vs", "shaders/shadow_cast.fs");

	// debug
	auto debugLightShader = Resources::loadShader("debug light", "shaders/light.vs", "shaders/light.fs");
	debugLightMaterial = new Material(debugLightShader);
  }

}// namespace primal::renderer
