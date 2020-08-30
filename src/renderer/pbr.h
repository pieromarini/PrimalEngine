#ifndef PBR_H
#define PBR_H

#include "core/math/linear_algebra/vector.h"
#include <vector>

namespace primal {
  class Entity;
}

namespace primal::renderer {

  class Renderer;
  class Material;
  class Mesh;
  class RenderTarget;
  class Texture;
  class TextureCube;
  class PBRCapture;
  class Shader;
  class PostProcessor;

  class PBR {
	public:
	  PBR(Renderer* renderer);
	  ~PBR();

	  // sets the combined irradiance/pre-filter global environment skylight
	  void SetSkyCapture(PBRCapture* capture);

	  // adds a processed PBR capture to the list of irradiance probes
	  void AddIrradianceProbe(PBRCapture* capture, math::vec3 position, float radius);

	  // removes all irradiance probe entries from the global GI grid
	  void ClearIrradianceProbes();

	  // generate an irradiance and pre-filter map out of a 2D equirectangular map (preferably HDR)
	  PBRCapture* ProcessEquirectangular(Texture* envMap);

	  // generate an irradiance and pre-filter map out of a cubemap texture
	  PBRCapture* ProcessCube(TextureCube* capture, bool prefilter = true);

	  // retrieves the environment skylight
	  PBRCapture* GetSkyCapture();

	  // retrieve all pushed irradiance probes
	  std::vector<PBRCapture*> GetIrradianceProbes(math::vec3 queryPos, float queryRadius);

	  // renders all reflection/irradiance probes for visualization/debugging.
	  void RenderProbes();

	private:
	  friend Renderer;
	  friend PostProcessor;

	  Renderer* m_renderer;

	  std::vector<PBRCapture*> m_captureProbes;
	  PBRCapture* m_skyCapture;
	  RenderTarget* m_renderTargetBRDFLUT;

	  // pbr pre-processing (irradiance/pre-filter)
	  Material* m_PBRHdrToCubemap;
	  Material* m_PBRIrradianceCapture;
	  Material* m_PBRPrefilterCapture;
	  Material* m_PBRIntegrateBRDF;
	  Mesh* m_PBRCaptureCube;
	  Entity* m_sceneEnvCube;

	  // irradiance capture
	  Shader* m_probeCaptureShader;
	  Shader* m_probeCaptureBackgroundShader;

	  // debug
	  Mesh* m_probeDebugSphere;
	  Shader* m_probeDebugShader;
  };

}

#endif
