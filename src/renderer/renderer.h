#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/vector4.h"
#include "core/math/matrix4.h"

#include "shading/shader.h"
#include "command_buffer.h"
#include "pbr_capture.h"
#include "gl_cache.h"

#include "components/point_light.h"
#include "components/directional_light.h"

#include "resources/primitives/quad.h"

namespace primal::renderer {

  class Mesh;
  class Material;
  class Scene;
  class Entity;
  class Camera;
  class RenderTarget;
  class MaterialLibrary;
  class PBR;
  class PostProcessor;

  class Renderer {
	public:

	  bool IrradianceGI = true;
	  bool Shadows = true;
	  bool Lights = true;
	  bool RenderLights = true;
	  bool LightVolumes = false;
	  bool RenderProbes = false;
	  bool Wireframe = false;

	  Renderer();
	  ~Renderer();

	  void init(GLADloadproc loadProcFunc);

	  void SetRenderSize(unsigned int width, unsigned int height);
	  math::Vector2 GetRenderSize();

	  void SetTarget(RenderTarget* renderTarget, GLenum target = GL_TEXTURE_2D);

	  Camera* GetCamera();
	  void SetCamera(Camera* camera);

	  PostProcessor* GetPostProcessor();

	  // create either a deferred default material (based on default set of materials available (like glass)), or a custom material (with custom you have to supply your own shader)
	  Material* CreateMaterial(std::string base = "default");
	  Material* CreateCustomMaterial(Shader* shader);
	  Material* CreatePostProcessingMaterial(Shader* shader);

	  void PushRender(Mesh* mesh, Material* material, math::Matrix4 transform = math::Matrix4(), math::Matrix4 prevFrameTransform = math::Matrix4());
	  void PushRender(Entity* node);
	  void PushPostProcessor(Material* postProcessor);

	  void AddLight(DirectionalLight* light);
	  void AddLight(PointLight* light);

	  void RenderPushedCommands();

	  void Blit(Texture* src, RenderTarget* dst = nullptr, Material* material = nullptr, std::string textureUniformName = "TexSrc");

	  // pbr
	  void SetSkyCapture(PBRCapture* pbrEnvironment);
	  PBRCapture* GetSkypCature();
	  void AddIrradianceProbe(math::Vector3 position, float radius);
	  void BakeProbes(Entity* scene = nullptr);

	private:
	  friend PostProcessor;
	  friend PBR;

	  // render state
	  CommandBuffer* m_CommandBuffer{ nullptr };
	  GLCache m_GLCache;
	  math::Vector2 m_RenderSize;

	  // lighting
	  std::vector<DirectionalLight*> m_DirectionalLights;
	  std::vector<PointLight*> m_PointLights;
	  RenderTarget* m_GBuffer = nullptr;
	  Mesh* m_DeferredPointMesh;

	  // materials
	  MaterialLibrary* m_MaterialLibrary{ nullptr };

	  // camera
	  Camera* m_Camera{ nullptr };
	  math::Matrix4 m_PrevViewProjection;

	  // render-targets/post
	  std::vector<RenderTarget*> m_RenderTargetsCustom;
	  RenderTarget* m_CurrentRenderTargetCustom = nullptr;
	  RenderTarget* m_CustomTarget{ nullptr };
	  RenderTarget* m_PostProcessTarget1{ nullptr };
	  PostProcessor* m_PostProcessor{ nullptr };
	  Quad* m_NDCPlane{ nullptr };
	  unsigned int m_FramebufferCubemap;
	  unsigned int m_CubemapDepthRBO;

	  // shadow buffers
	  std::vector<RenderTarget*> m_ShadowRenderTargets;
	  std::vector<math::Matrix4> m_ShadowViewProjections;

	  // pbr
	  PBR* m_PBR;
	  unsigned int m_PBREnvironmentIndex;
	  std::vector<math::Vector4> m_ProbeSpatials;

	  // ubo
	  unsigned int m_GlobalUBO;

	  // debug
	  Mesh* m_DebugLightMesh{ nullptr };

	  // renderer-specific logic for rendering a custom (forward-pass) command
	  void renderCustomCommand(RenderCommand* command, Camera* customCamera, bool updateGLSettings = true);
	  // renderer-specific logic for rendering a list of commands to a target cubemap
	  void renderToCubemap(Entity* scene, TextureCube* target, math::Vector3 position = math::Vector3(0.0f), unsigned int mipLevel = 0);
	  void renderToCubemap(std::vector<RenderCommand>& renderCommands, TextureCube* target, math::Vector3 position = math::Vector3(0.0f), unsigned int mipLevel = 0);
	  // minimal render logic to render a mesh
	  void renderMesh(Mesh* mesh, Shader* shader);
	  // updates the global uniform buffer objects
	  void updateGlobalUBOs();
	  // returns the currently active render target
	  RenderTarget* getCurrentRenderTarget();

	  // deferred logic:
	  // renders all ambient lighting (including indirect IBL)
	  void renderDeferredAmbient();
	  // render directional light
	  void renderDeferredDirLight(DirectionalLight* light);
	  // render point light
	  void renderDeferredPointLight(PointLight* light);

	  // render mesh for shadow buffer generation
	  void renderShadowCastCommand(RenderCommand* command, const math::Matrix4& projection, const math::Matrix4& view);
  };

}

#endif
