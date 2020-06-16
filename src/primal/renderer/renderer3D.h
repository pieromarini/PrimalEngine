#pragma once

#include "perspectiveCamera.h"

#include "texture.h"
#include "primal/model/model.h"
#include "primal/renderer/framebuffer.h"
#include "primal/renderer/shadowMap.h"

namespace primal {

  struct Renderer3DStorage {
	ref_ptr<Shader> textureShader;
	ref_ptr<Shader> screenShader;
	ref_ptr<Shader> depthShader;
	ref_ptr<Shader> debugDepthShader;
	ref_ptr<Framebuffer> framebuffer;
	ref_ptr<ShadowMap> shadowMapFBO;
	ref_ptr<Framebuffer> depthDebugFBO;
	ref_ptr<VertexArray> screenQuadVA;
	// NOTE: temporary. used for shadow mapping rendering. Will be moved to it's own class later.
	glm::mat4 lightSpaceMatrix{};
  };

  class Renderer3D {
	public:
	  static void init();
	  static void shutdown();

	  static void beginScene(const PerspectiveCamera& camera);
	  static void endScene();

	  // Primitives
	  static void drawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
	  static void drawCube(const glm::vec3& position, const glm::vec3& size, const ref_ptr<Texture2D>& texture);

	  static void drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size, const ref_ptr<Texture2D>& texture, const ref_ptr<Shader> shader);
	  static void drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size, const ref_ptr<Shader> shader);

	  static void beginShadowMap();
	  static void endShadowMap();
	  static void debugDepthMap();

	  static ref_ptr<Shader> depthShader();
	  static ref_ptr<Shader> textureShader();

	  static void drawScreenQuad();

	  static glm::vec3 lightDirection;
	  static glm::vec3 pointLightPosition;
	  static glm::vec3 pointLightPosition2;
	  static uint32_t sceneTextureTarget;
	  static uint32_t depthTextureTarget;
  };

}
