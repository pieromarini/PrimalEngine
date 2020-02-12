#pragma once

#include "perspectiveCamera.h"

#include "texture.h"
#include "../model/model.h"

namespace primal {

  class Renderer3D {
	public:
	  static void init();
	  static void shutdown();

	  static void beginScene(const PerspectiveCamera& camera);
	  static void endScene();

	  // Primitives
	  static void drawCube(const glm::vec3& position, const glm::vec3& size, const glm::vec4& color);
	  static void drawCube(const glm::vec3& position, const glm::vec3& size, const ref_ptr<Texture2D>& texture);

	  static void drawModel(const ref_ptr<Model>& model, const glm::vec3& position, const glm::vec3& size);
  };

}
