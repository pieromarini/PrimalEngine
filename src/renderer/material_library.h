#ifndef MATERIAL_LIBRARY_H
#define MATERIAL_LIBRARY_H

#include "util/sid.h"
#include <string>
#include <vector>
#include <map>

namespace primal::renderer {

  class Material;
  class Renderer;
  class Shader;
  class RenderTarget;

  class MaterialLibrary {
	public:
	  MaterialLibrary(RenderTarget* gBuffer);
	  ~MaterialLibrary();

	  Material* CreateMaterial(std::string base);
	  Material* CreateCustomMaterial(Shader* shader);
	  Material* CreatePostProcessingMaterial(Shader* shader);
	private:
	  void generateDefaultMaterials();
	  void generateInternalMaterials(RenderTarget* gBuffer);

	  std::map<StringId, Material*> m_defaultMaterials;

	  std::vector<Material*> m_materials;

	  // NOTE: internal render-specific materials
	  Material* defaultBlitMaterial;

	  Shader* deferredAmbientShader;
	  Shader* deferredIrradianceShader;
	  Shader* deferredDirectionalShader;
	  Shader* deferredPointShader;

	  Shader* dirShadowShader;

	  Material* debugLightMaterial;

	  friend Renderer;
  };

}

#endif
