#include <stack>
#include <imgui.h>

#include "renderer.h"

#include "scene/scene.h"
#include "components/camera_component.h"

#include "resources/mesh.h"
#include "resources/primitives/cube.h"
#include "resources/resources.h"

#include "render_target.h"
#include "post_processor.h"
#include "pbr.h"
#include "material_library.h"

#include "shading/material.h"

#include "tools/log.h"
#include "util/sid.h"


namespace primal::renderer {

  Renderer::Renderer() {
  }

  Renderer::~Renderer() {
	delete m_CommandBuffer;

	delete m_NDCPlane;

	delete m_MaterialLibrary;

	delete m_GBuffer;
	delete m_CustomTarget;

	// shadows
	for (int i = 0; i < m_ShadowRenderTargets.size(); ++i) {
	  delete m_ShadowRenderTargets[i];
	}

	// lighting
	delete m_DebugLightMesh;

	// post-processing
	delete m_PostProcessTarget1;
	delete m_PostProcessor;

	// pbr
	delete m_PBR;
  }

  void Renderer::init(GLADloadproc loadProcFunc) {
	// initialize render items
	m_CommandBuffer = new CommandBuffer(this);

	// configure default OpenGL state
	m_GLCache.setDepthTest(true);
	m_GLCache.setCull(true);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glViewport(0.0f, 0.0f, m_RenderSize.x, m_RenderSize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	m_NDCPlane = new Quad{};
	glGenFramebuffers(1, &m_FramebufferCubemap);
	glGenRenderbuffers(1, &m_CubemapDepthRBO);

	m_CustomTarget = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, true);
	m_PostProcessTarget1 = new RenderTarget(1, 1, GL_UNSIGNED_BYTE, 1, false);
	m_PostProcessor = new PostProcessor(this);

	// lights
	m_DebugLightMesh = new Sphere(16, 16);
	m_DeferredPointMesh = new Sphere(16, 16);

	// deferred renderer
	m_GBuffer = new RenderTarget(1, 1, GL_HALF_FLOAT, 4, true);

	// materials
	m_MaterialLibrary = new MaterialLibrary(m_GBuffer);

	// shadows
	for (int i = 0; i < 4; ++i)// allow up to a total of 4 dir/spot shadow casters
	{
	  RenderTarget* rt = new RenderTarget(2048, 2048, GL_UNSIGNED_BYTE, 1, true);
	  rt->m_DepthStencil.bind();
	  rt->m_DepthStencil.setFilterMin(GL_NEAREST);
	  rt->m_DepthStencil.setFilterMax(GL_NEAREST);
	  rt->m_DepthStencil.setWrapMode(GL_CLAMP_TO_BORDER);
	  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	  m_ShadowRenderTargets.push_back(rt);
	}

	// pbr
	m_PBR = new PBR(this);

	// ubo
	glGenBuffers(1, &m_GlobalUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
	glBufferData(GL_UNIFORM_BUFFER, 720, nullptr, GL_STREAM_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GlobalUBO);

	// default PBR pre-compute (get a more default oriented HDR map for this)
	Texture* hdrMap = Resources::loadHDR("sky env", "textures/backgrounds/alley.hdr");
	PBRCapture* envBridge = m_PBR->processEquirectangular(hdrMap);
	setSkyCapture(envBridge);
  }

  void Renderer::setRenderSize(unsigned int width, unsigned int height) {
	m_RenderSize.x = width;
	m_RenderSize.y = height;

	m_GBuffer->resize(width, height);

	m_CustomTarget->resize(width, height);
	m_PostProcessTarget1->resize(width, height);

	m_PostProcessor->updateRenderSize(width, height);
  }

  math::vec2 Renderer::getRenderSize() {
	return m_RenderSize;
  }

  void Renderer::setTarget(RenderTarget* renderTarget, GLenum target) {
	m_CurrentRenderTargetCustom = renderTarget;
	if (renderTarget != nullptr) {
	  if (std::find(m_RenderTargetsCustom.begin(), m_RenderTargetsCustom.end(), renderTarget) == m_RenderTargetsCustom.end()) {
		m_RenderTargetsCustom.push_back(renderTarget);
	  }
	}
  }

  Camera* Renderer::getCamera() {
	return m_Camera;
  }

  void Renderer::setCamera(Camera* camera) {
	m_Camera = camera;
  }

  PostProcessor* Renderer::getPostProcessor() {
	return m_PostProcessor;
  }

  Material* Renderer::createMaterial(std::string base) {
	return m_MaterialLibrary->createMaterial(base);
  }

  Material* Renderer::createCustomMaterial(Shader* shader) {
	return m_MaterialLibrary->createCustomMaterial(shader);
  }

  Material* Renderer::createPostProcessingMaterial(Shader* shader) {
	return m_MaterialLibrary->createPostProcessingMaterial(shader);
  }

  void Renderer::pushRender(Mesh* mesh, Material* material, math::Matrix4 transform, math::Matrix4 prevFrameTransform) {
	// get current render target
	RenderTarget* target = getCurrentRenderTarget();
	// don't render right away but push to the command buffer for later rendering.
	m_CommandBuffer->push(mesh, material, transform, prevFrameTransform, math::vec3(-99999.0f), math::vec3(99999.0f), target);
  }

  void Renderer::pushRender(Entity* node) {
	// update transform(s) before pushing node to render command buffer
	node->updateTransform(true);

	// get current render target
	RenderTarget* target = getCurrentRenderTarget();
	// traverse through all the scene nodes and for each node: push its render state to the
	// command buffer together with a calculated transform matrix.
	std::stack<Entity*> nodeStack;
	nodeStack.push(node);
	for (unsigned int i = 0; i < node->getChildCount(); ++i)
	  nodeStack.push(node->getChildByIndex(i));
	while (!nodeStack.empty()) {
	  Entity* node = nodeStack.top();
	  nodeStack.pop();
	  // only push render command if the child isn't a container node.
	  if (node->mesh) {
		math::vec3 boxMinWorld = node->getWorldPosition() + (node->getWorldScale() * node->boxMin);
		math::vec3 boxMaxWorld = node->getWorldPosition() + (node->getWorldScale() * node->boxMax);
		m_CommandBuffer->push(node->mesh, node->material, node->getTransform(), node->getPrevTransform(), boxMinWorld, boxMaxWorld, target);
	  }
	  for (unsigned int i = 0; i < node->getChildCount(); ++i)
		nodeStack.push(node->getChildByIndex(i));
	}
  }

  void Renderer::pushPostProcessor(Material* postProcessor) {
	// we only care about the material, mesh as NDC quad is pre-defined.
	m_CommandBuffer->push(nullptr, postProcessor);
  }

  void Renderer::addLight(DirectionalLight* light) {
	m_DirectionalLights.push_back(light);
  }

  void Renderer::addLight(PointLight* light) {
	m_PointLights.push_back(light);
  }

  void Renderer::renderPushedCommands() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* 

	   General outline of all the render steps/passes:
	   - First we render all pushed geometry to the GBuffer.
	   - We then render all shadow casting geometry to the shadow buffer.
	   - Pre-lighting post-processing steps.
	   - Deferred lighting pass (ambient, directional, point).
	   - Process deferred data s.t. forward pass is neatly integrated with deferred pass.
	   - Then we do the forward 'custom' rendering pass where developers can write their 
	   own shaders and render stuff as they want, not sacrifcing flexibility; this 
	   includes custom render targets.
	   - Then we render all alpha blended materials last.
	   - Then we do post-processing, one can supply their own post-processing materials 
	   by setting the 'post-processing' flag of the material.
	   Each material flagged as post-processing will run after the default post-processing 
	   shaders (before/after HDR-tonemap/gamma-correct).
	   */
	// sort all pushed render commands by heavy state-switches e.g. shader switches.
	m_CommandBuffer->sort();

	// update (global) uniform buffers
	updateGlobalUBOs();

	// set default GL state
	m_GLCache.setBlend(false);
	m_GLCache.setCull(true);
	m_GLCache.setCullFace(GL_BACK);
	m_GLCache.setDepthTest(true);
	m_GLCache.setDepthFunc(GL_LESS);

	// 1. Geometry buffer
	std::vector<RenderCommand> deferredRenderCommands = m_CommandBuffer->getDeferredRenderCommands(true);
	glViewport(0, 0, m_RenderSize.x, m_RenderSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, m_GBuffer->m_id);
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_GLCache.setPolygonMode(Wireframe ? GL_LINE : GL_FILL);
	for (unsigned int i = 0; i < deferredRenderCommands.size(); ++i) {
	  renderCustomCommand(&deferredRenderCommands[i], nullptr, false);
	}
	m_GLCache.SetPolygonMode(GL_FILL);

	//attachments[0] = GL_NONE; // disable for next pass (shadow map generation)
	attachments[1] = GL_NONE;
	attachments[2] = GL_NONE;
	attachments[3] = GL_NONE;
	glDrawBuffers(4, attachments);

	// 2. render all shadow casters to light shadow buffers
	if (Shadows) {
	  m_GLCache.setCullFace(GL_FRONT);
	  std::vector<RenderCommand> shadowRenderCommands = m_CommandBuffer->getShadowCastRenderCommands();
	  m_ShadowViewProjections.clear();

	  unsigned int shadowRtIndex = 0;
	  for (int i = 0; i < m_DirectionalLights.size(); ++i) {
		DirectionalLight* light = m_DirectionalLights[i];
		if (light->CastShadows) {
		  m_MaterialLibrary->dirShadowShader->Use();

		  glBindFramebuffer(GL_FRAMEBUFFER, m_ShadowRenderTargets[shadowRtIndex]->m_id);
		  glViewport(0, 0, m_ShadowRenderTargets[shadowRtIndex]->Width, m_ShadowRenderTargets[shadowRtIndex]->Height);
		  glClear(GL_DEPTH_BUFFER_BIT);

		  math::Matrix4 lightProjection = math::orthographic(-20.0f, 20.0f, 20.0f, -20.0f, -15.0f, 20.0f);
		  math::Matrix4 lightView = math::lookAt(-light->Direction * 10.0f, math::vec3(0.0), math::vec3::UP);
		  m_DirectionalLights[i]->LightSpaceViewProjection = lightProjection * lightView;
		  m_DirectionalLights[i]->ShadowMapRT = m_ShadowRenderTargets[shadowRtIndex];
		  for (int j = 0; j < shadowRenderCommands.size(); ++j) {
			renderShadowCastCommand(&shadowRenderCommands[j], lightProjection, lightView);
		  }
		  ++shadowRtIndex;
		}
	  }
	  m_GLCache.setCullFace(GL_BACK);
	}
	attachments[0] = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(4, attachments);

	// 3. do post-processing steps before lighting stage (e.g. SSAO)
	m_PostProcessor->processPreLighting(this, m_GBuffer, m_Camera);

	glBindFramebuffer(GL_FRAMEBUFFER, m_CustomTarget->m_id);
	glViewport(0, 0, m_CustomTarget->Width, m_CustomTarget->Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// 4. Render deferred shader for each light (full quad for directional, spheres for point lights)
	m_GLCache.setDepthTest(false);
	m_GLCache.setBlend(true);
	m_GLCache.setBlendFunc(GL_ONE, GL_ONE);

	// bind gbuffer
	m_GBuffer->getColorTexture(0)->bind(0);
	m_GBuffer->getColorTexture(1)->bind(1);
	m_GBuffer->getColorTexture(2)->bind(2);

	// ambient lighting
	renderDeferredAmbient();

	if (Lights) {
	  // directional lights
	  for (auto it = m_DirectionalLights.begin(); it != m_DirectionalLights.end(); ++it) {
		renderDeferredDirLight(*it);
	  }
	  // point lights
	  m_GLCache.setCullFace(GL_FRONT);
	  for (auto it = m_PointLights.begin(); it != m_PointLights.end(); ++it) {
		// only render point lights if within frustum
		if (m_Camera->frustum.intersect((*it)->Position, (*it)->Radius)) {
		  renderDeferredPointLight(*it);
		}
	  }
	  m_GLCache.SetCullFace(GL_BACK);
	}

	m_GLCache.setDepthTest(true);
	m_GLCache.setBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	m_GLCache.setBlend(false);

	// 5. blit depth buffer to default for forward rendering
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_GBuffer->m_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_CustomTarget->m_id);// write to default framebuffer
	glBlitFramebuffer(
		0, 0, m_GBuffer->Width, m_GBuffer->Height, 0, 0, m_RenderSize.x, m_RenderSize.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	// 6. custom forward render pass
	// push default render target to the end of the render target buffer s.t. we always render
	// the default buffer last.
	m_RenderTargetsCustom.push_back(nullptr);
	for (unsigned int targetIndex = 0; targetIndex < m_RenderTargetsCustom.size(); ++targetIndex) {
	  RenderTarget* renderTarget = m_RenderTargetsCustom[targetIndex];
	  if (renderTarget) {
		glViewport(0, 0, renderTarget->Width, renderTarget->Height);
		glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->m_id);
		if (renderTarget->HasDepthAndStencil)
		  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		else
		  glClear(GL_COLOR_BUFFER_BIT);
		m_Camera->SetPerspective(m_Camera->FOV,
			(float)renderTarget->Width / (float)renderTarget->Height,
			0.1,
			100.0f);
	  } else {
		// don't render to default framebuffer, but to custom target framebuffer which
		// we'll use for post-processing.
		glViewport(0, 0, m_RenderSize.x, m_RenderSize.y);
		glBindFramebuffer(GL_FRAMEBUFFER, m_CustomTarget->m_id);
		m_Camera->SetPerspective(m_Camera->FOV, m_RenderSize.x / m_RenderSize.y, 0.1, 100.0f);
	  }

	  // sort all render commands and retrieve the sorted array
	  std::vector<RenderCommand> renderCommands = m_CommandBuffer->GetCustomRenderCommands(renderTarget);

	  // terate over all the render commands and execute
	  m_GLCache.SetPolygonMode(Wireframe ? GL_LINE : GL_FILL);
	  for (unsigned int i = 0; i < renderCommands.size(); ++i) {
		renderCustomCommand(&renderCommands[i], nullptr);
	  }
	  m_GLCache.SetPolygonMode(GL_FILL);
	}

	// 7. alpha material pass
	glViewport(0, 0, m_RenderSize.x, m_RenderSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, m_CustomTarget->m_id);
	std::vector<RenderCommand> alphaRenderCommands = m_CommandBuffer->GetAlphaRenderCommands(true);
	for (unsigned int i = 0; i < alphaRenderCommands.size(); ++i) {
	  renderCustomCommand(&alphaRenderCommands[i], nullptr);
	}

	// render light mesh (as visual cue), if requested
	for (auto it = m_PointLights.begin(); it != m_PointLights.end(); ++it) {
	  if ((*it)->RenderMesh) {
		m_MaterialLibrary->debugLightMaterial->SetVector("lightColor", (*it)->Color * (*it)->Intensity * 0.25f);

		RenderCommand command;
		command.material = m_MaterialLibrary->debugLightMaterial;
		command.mesh = m_DebugLightMesh;
		math::Matrix4 model;
		math::translate(model, (*it)->Position);
		math::scale(model, math::vec3(0.25f));
		command.transform = model;

		renderCustomCommand(&command, nullptr);
	  }
	}

	// 8. post-processing stage after all lighting calculations
	m_PostProcessor->ProcessPostLighting(this, m_GBuffer, m_CustomTarget, m_Camera);

	// 9. render debug visuals
	glViewport(0, 0, m_RenderSize.x, m_RenderSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, m_CustomTarget->m_id);
	if (LightVolumes) {
	  m_GLCache.SetPolygonMode(GL_LINE);
	  m_GLCache.SetCullFace(GL_FRONT);
	  for (auto it = m_PointLights.begin(); it != m_PointLights.end(); ++it) {
		m_MaterialLibrary->debugLightMaterial->SetVector("lightColor", (*it)->Color);

		RenderCommand command;
		command.material = m_MaterialLibrary->debugLightMaterial;
		command.mesh = m_DebugLightMesh;
		math::Matrix4 model;
		math::translate(model, (*it)->Position);
		math::scale(model, math::vec3((*it)->Radius));
		command.transform = model;

		renderCustomCommand(&command, nullptr);
	  }
	  m_GLCache.SetPolygonMode(GL_FILL);
	  m_GLCache.SetCullFace(GL_BACK);
	}
	if (RenderProbes) {
	  m_PBR->RenderProbes();
	}

	// 10. custom post-processing pass
	std::vector<RenderCommand> postProcessingCommands = m_CommandBuffer->GetPostProcessingRenderCommands();
	for (unsigned int i = 0; i < postProcessingCommands.size(); ++i) {
	  // ping-pong between render textures
	  bool even = i % 2 == 0;
	  Blit(even ? m_CustomTarget->GetColorTexture(0) : m_PostProcessTarget1->GetColorTexture(0),
		  even ? m_PostProcessTarget1 : m_CustomTarget,
		  postProcessingCommands[i].material);
	}

	// 11. final post-processing steps, blitting to default framebuffer
	m_PostProcessor->Blit(this, postProcessingCommands.size() % 2 == 0 ? m_CustomTarget->GetColorTexture(0) : m_PostProcessTarget1->GetColorTexture(0));

	// store view projection as previous view projection for next frame's motion blur
	m_PrevViewProjection = m_Camera->Projection * m_Camera->View;

	// clear the command buffer s.t. the next frame/call can start from an empty slate again.
	m_CommandBuffer->Clear();

	// clear render state
	m_RenderTargetsCustom.clear();
	m_CurrentRenderTargetCustom = nullptr;
  }
  // ------------------------------------------------------------------------
  void Renderer::Blit(Texture* src,
	  RenderTarget* dst,
	  Material* material,
	  std::string textureUniformName) {
	// if a destination target is given, bind to its framebuffer
	if (dst) {
	  glViewport(0, 0, dst->Width, dst->Height);
	  glBindFramebuffer(GL_FRAMEBUFFER, dst->m_id);
	  if (dst->HasDepthAndStencil)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	  else
		glClear(GL_COLOR_BUFFER_BIT);
	}
	// else we bind to the default framebuffer
	else {
	  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	  glViewport(0, 0, m_RenderSize.x, m_RenderSize.y);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	// if no material is given, use default blit material
	if (!material) {
	  material = m_MaterialLibrary->defaultBlitMaterial;
	}
	// if a source render target is given, use its color buffer as input to material shader.
	if (src) {
	  material->SetTexture(textureUniformName, src, 0);
	}
	// render screen-space material to quad which will be displayed in dst's buffers.
	RenderCommand command;
	command.material = material;
	command.mesh = m_NDCPlane;
	renderCustomCommand(&command, nullptr);
  }
  // ------------------------------------------------------------------------
  void Renderer::SetSkyCapture(PBRCapture* pbrEnvironment) {
	m_PBR->SetSkyCapture(pbrEnvironment);
  }
  // ------------------------------------------------------------------------
  PBRCapture* Renderer::GetSkypCature() {
	return m_PBR->GetSkyCapture();
  }
  // ------------------------------------------------------------------------
  void Renderer::AddIrradianceProbe(math::vec3 position, float radius) {
	m_ProbeSpatials.push_back(math::vec4(position, radius));
  }
  // ------------------------------------------------------------------------
  void Renderer::BakeProbes(Entity* scene) {
	if (!scene) {
	  // if no scene node was provided, use root node (capture all)
	  scene = Scene::Root;
	}
	scene->UpdateTransform();
	// build a command list of nodes within the reflection probe's capture box/radius.
	CommandBuffer commandBuffer(this);
	std::vector<Material*> materials;

	// originally a recursive function but transformed to iterative version
	std::stack<Entity*> sceneStack;
	sceneStack.push(scene);
	while (!sceneStack.empty()) {
	  Entity* node = sceneStack.top();
	  sceneStack.pop();
	  if (node->mesh != nullptr) {
		auto samplerUniforms = *(node->material->GetSamplerUniforms());
		if (samplerUniforms.find("TexAlbedo") != samplerUniforms.end()) {
		  materials.push_back(new Material(m_PBR->m_ProbeCaptureShader));
		  materials[materials.size() - 1]->SetTexture("TexAlbedo", samplerUniforms["TexAlbedo"].texture, 0);
		  if (samplerUniforms.find("TexNormal") != samplerUniforms.end()) {
			materials[materials.size() - 1]->SetTexture("TexNormal", samplerUniforms["TexNormal"].texture, 1);
		  }
		  if (samplerUniforms.find("TexMetallic") != samplerUniforms.end()) {
			materials[materials.size() - 1]->SetTexture("TexMetallic", samplerUniforms["TexMetallic"].texture, 2);
		  }
		  if (samplerUniforms.find("TexRoughness") != samplerUniforms.end()) {
			materials[materials.size() - 1]->SetTexture("TexRoughness", samplerUniforms["TexRoughness"].texture, 3);
		  }
		  commandBuffer.Push(node->mesh, materials[materials.size() - 1], node->GetTransform());
		} else if (samplerUniforms.find("background") != samplerUniforms.end()) {// we have a background scene node, add those as well
		  materials.push_back(new Material(m_PBR->m_ProbeCaptureBackgroundShader));
		  materials[materials.size() - 1]->SetTextureCube("background", samplerUniforms["background"].textureCube, 0);
		  materials[materials.size() - 1]->DepthCompare = node->material->DepthCompare;
		  commandBuffer.Push(node->mesh, materials[materials.size() - 1], node->GetTransform());
		}
	  }
	  for (unsigned int i = 0; i < node->GetChildCount(); ++i)
		sceneStack.push(node->GetChildByIndex(i));
	}
	commandBuffer.Sort();
	std::vector<RenderCommand> renderCommands = commandBuffer.GetCustomRenderCommands(nullptr);

	m_PBR->ClearIrradianceProbes();
	for (int i = 0; i < m_ProbeSpatials.size(); ++i) {
	  TextureCube renderResult;
	  renderResult.DefaultInitialize(32, 32, GL_RGB, GL_FLOAT);

	  renderToCubemap(renderCommands, &renderResult, m_ProbeSpatials[i].xyz);

	  PBRCapture* capture = m_PBR->ProcessCube(&renderResult, false);
	  m_PBR->AddIrradianceProbe(capture, m_ProbeSpatials[i].xyz, m_ProbeSpatials[i].w);
	}

	for (int i = 0; i < materials.size(); ++i) {
	  delete materials[i];
	}
  }
  // ------------------------------------------------------------------------
  void Renderer::renderCustomCommand(RenderCommand* command, Camera* customCamera, bool updateGLSettings) {
	Material* material = command->material;
	Mesh* mesh = command->mesh;

	// update global GL blend state based on material
	if (updateGLSettings) {
	  m_GLCache.SetBlend(material->Blend);
	  if (material->Blend) {
		m_GLCache.SetBlendFunc(material->BlendSrc, material->BlendDst);
	  }
	  m_GLCache.SetDepthFunc(material->DepthCompare);
	  m_GLCache.SetDepthTest(material->DepthTest);
	  m_GLCache.SetCull(material->Cull);
	  m_GLCache.SetCullFace(material->CullFace);
	}

	// default uniforms that are always configured regardless of shader configuration (see them
	// as a default set of shader uniform variables always there); with UBO
	material->GetShader()->Use();
	if (customCamera)// pass custom camera specific uniform
	{
	  material->GetShader()->SetMatrix("projection", customCamera->Projection);
	  material->GetShader()->SetMatrix("view", customCamera->View);
	  material->GetShader()->SetVector("CamPos", customCamera->Position);
	}
	material->GetShader()->SetMatrix("model", command->transform);
	material->GetShader()->SetMatrix("prevModel", command->prevTransform);

	material->GetShader()->SetBool("ShadowsEnabled", Shadows);
	if (Shadows && material->Type == MATERIAL_CUSTOM && material->ShadowReceive) {
	  for (int i = 0; i < m_DirectionalLights.size(); ++i) {
		if (m_DirectionalLights[i]->ShadowMapRT != nullptr) {
		  material->GetShader()->SetMatrix("lightShadowViewProjection" + std::to_string(i + 1), m_DirectionalLights[i]->LightSpaceViewProjection);
		  m_DirectionalLights[i]->ShadowMapRT->GetDepthStencilTexture()->Bind(10 + i);
		}
	  }
	}

	// bind/active uniform sampler/texture objects
	auto* samplers = material->GetSamplerUniforms();
	for (auto it = samplers->begin(); it != samplers->end(); ++it) {
	  if (it->second.Type == SHADER_TYPE_SAMPLERCUBE)
		it->second.textureCube->Bind(it->second.Unit);
	  else
		it->second.texture->Bind(it->second.Unit);
	}

	// set uniform state of material
	auto* uniforms = material->GetUniforms();
	for (auto it = uniforms->begin(); it != uniforms->end(); ++it) {
	  switch (it->second.Type) {
		case SHADER_TYPE_BOOL:
		  material->GetShader()->SetBool(it->first, it->second.Bool);
		  break;
		case SHADER_TYPE_INT:
		  material->GetShader()->SetInt(it->first, it->second.Int);
		  break;
		case SHADER_TYPE_FLOAT:
		  material->GetShader()->SetFloat(it->first, it->second.Float);
		  break;
		case SHADER_TYPE_VEC2:
		  material->GetShader()->SetVector(it->first, it->second.Vec2);
		  break;
		case SHADER_TYPE_VEC3:
		  material->GetShader()->SetVector(it->first, it->second.Vec3);
		  break;
		case SHADER_TYPE_VEC4:
		  material->GetShader()->SetVector(it->first, it->second.Vec4);
		  break;
		case SHADER_TYPE_MAT2:
		  material->GetShader()->SetMatrix(it->first, it->second.Mat2);
		  break;
		case SHADER_TYPE_MAT3:
		  material->GetShader()->SetMatrix(it->first, it->second.Mat3);
		  break;
		case SHADER_TYPE_MAT4:
		  material->GetShader()->SetMatrix(it->first, it->second.Mat4);
		  break;
		default:
		  Log::Message("Unrecognized Uniform type set.", LOG_ERROR);
		  break;
	  }
	}

	renderMesh(mesh, material->GetShader());
  }

  void Renderer::renderToCubemap(Entity* scene, TextureCube* target, math::vec3 position, unsigned int mipLevel) {

	// create a command buffer specifically for this operation (as to not conflict with main
	// command buffer)
	CommandBuffer commandBuffer(this);
	commandBuffer.push(scene->mesh, scene->material, scene->getTransform());

	std::stack<Entity*> childStack;
	for (unsigned int i = 0; i < scene->getChildCount(); ++i)
	  childStack.push(scene->getChildByIndex(i));
	while (!childStack.empty()) {
	  Entity* child = childStack.top();
	  childStack.pop();
	  commandBuffer.Push(child->mesh, child->material, child->getTransform());
	  for (unsigned int i = 0; i < child->getChildCount(); ++i)
		childStack.push(child->GetChildByIndex(i));
	}
	commandBuffer.sort();
	std::vector<RenderCommand> renderCommands = commandBuffer.getCustomRenderCommands(nullptr);

	renderToCubemap(renderCommands, target, position, mipLevel);
  }

  void Renderer::renderToCubemap(std::vector<RenderCommand>& renderCommands, TextureCube* target, math::vec3 position, unsigned int mipLevel) {
	// define 6 camera directions/lookup vectors
	Camera faceCameras[6] = {
	  Camera(position, math::vec3(1.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f)),
	  Camera(position, math::vec3(-1.0f, 0.0f, 0.0f), math::vec3(0.0f, -1.0f, 0.0f)),
	  Camera(position, math::vec3(0.0f, 1.0f, 0.0f), math::vec3(0.0f, 0.0f, 1.0f)),
	  Camera(position, math::vec3(0.0f, -1.0f, 0.0f), math::vec3(0.0f, 0.0f, -1.0f)),
	  Camera(position, math::vec3(0.0f, 0.0f, 1.0f), math::vec3(0.0f, -1.0f, 0.0f)),
	  Camera(position, math::vec3(0.0f, 0.0f, -1.0f), math::vec3(0.0f, -1.0f, 0.0f))
	};

	// resize target dimensions based on mip level we're rendering.
	float width = (float)target->FaceWidth * std::pow(0.5, mipLevel);
	float height = (float)target->FaceHeight * std::pow(0.5, mipLevel);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferCubemap);
	glBindRenderbuffer(GL_RENDERBUFFER, m_CubemapDepthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CubemapDepthRBO);

	// resize relevant buffers
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferCubemap);

	for (unsigned int i = 0; i < 6; ++i) {
	  Camera* camera = &faceCameras[i];
	  camera->SetPerspective(math::Util::deg2Rad(90.0f), width / height, 0.1f, 100.0f);
	  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, target->m_id, mipLevel);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	  for (unsigned int i = 0; i < renderCommands.size(); ++i) {
		// cubemap generation only works w/ custom materials
		assert(renderCommands[i].material->Type == MATERIAL_CUSTOM);
		renderCustomCommand(&renderCommands[i], camera);
	  }
	}
  }

  void Renderer::renderMesh(Mesh* mesh, Shader* shader) {
	glBindVertexArray(mesh->m_VAO);
	if (mesh->indices.size() > 0) {
	  glDrawElements(mesh->topology == TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, mesh->Indices.size(), GL_UNSIGNED_INT, 0);
	} else {
	  glDrawArrays(mesh->topology == TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh->Positions.size());
	}
  }

  void Renderer::updateGlobalUBOs() {
	glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
	// transformation matrices
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(math::Matrix4), &(m_Camera->projection * m_Camera->view)[0][0]); // sizeof(math::Matrix4) = 64 bytes
	glBufferSubData(GL_UNIFORM_BUFFER, 64, sizeof(math::Matrix4), &m_PrevViewProjection[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 128, sizeof(math::Matrix4), &m_Camera->projection[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 192, sizeof(math::Matrix4), &m_Camera->view[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, 256, sizeof(math::Matrix4), &m_Camera->view[0][0]); // TODO: make inv function in math library
	// scene data
	glBufferSubData(GL_UNIFORM_BUFFER, 320, sizeof(math::vec4), &m_Camera->Position[0]);
	// lighting
	unsigned int stride = 2 * sizeof(math::vec4);
	for (unsigned int i = 0; i < m_DirectionalLights.size() && i < 4; ++i)// no more than 4 directional lights
	{
	  glBufferSubData(GL_UNIFORM_BUFFER, 336 + i * stride, sizeof(math::vec4), &m_DirectionalLights[i]->Direction[0]);
	  glBufferSubData(GL_UNIFORM_BUFFER, 336 + i * stride + sizeof(math::vec4), sizeof(math::vec4), &m_DirectionalLights[i]->Color[0]);
	}
	for (unsigned int i = 0; i < m_PointLights.size() && i < 8; ++i)//  constrained to max 8 point lights in forward context
	{
	  glBufferSubData(GL_UNIFORM_BUFFER, 464 + i * stride, sizeof(math::vec4), &m_PointLights[i]->Position[0]);
	  glBufferSubData(GL_UNIFORM_BUFFER, 464 + i * stride + sizeof(math::vec4), sizeof(math::vec4), &m_PointLights[i]->Color[0]);
	}
  }

  RenderTarget* Renderer::getCurrentRenderTarget() {
	return m_CurrentRenderTargetCustom;
  }

  void Renderer::renderDeferredAmbient() {
	PBRCapture* skyCapture = m_PBR->getSkyCapture();
	auto irradianceProbes = m_PBR->m_CaptureProbes;

	// if irradiance probes are present, use these as ambient lighting
	if (IrradianceGI && irradianceProbes.size() > 0) {
	  skyCapture->Prefiltered->bind(4);
	  m_PBR->m_RenderTargetBRDFLUT->getColorTexture(0)->bind(5);
	  m_PostProcessor->SSAOOutput->bind(6);

	  m_GLCache.setCullFace(GL_FRONT);
	  for (int i = 0; i < irradianceProbes.size(); ++i) {
		PBRCapture* probe = irradianceProbes[i];
		// only render probe if within frustum
		if (m_Camera->Frustum.intersect(probe->Position, probe->Radius)) {
		  probe->Irradiance->bind(3);

		  Shader* irradianceShader = m_MaterialLibrary->deferredIrradianceShader;
		  irradianceShader->use();
		  irradianceShader->setVector("camPos", m_Camera->Position);
		  irradianceShader->setVector("probePos", probe->Position);
		  irradianceShader->setFloat("probeRadius", probe->Radius);
		  irradianceShader->setInt("SSAO", m_PostProcessor->SSAO);

		  math::Matrix4 model;
		  math::translate(model, probe->Position);
		  math::scale(model, math::vec3(probe->Radius));
		  irradianceShader->setMatrix("model", model);

		  renderMesh(m_DeferredPointMesh, irradianceShader);
		}
	  }
	  m_GLCache.setCullFace(GL_BACK);
	} else {
	  skyCapture->irradiance->bind(3);
	  skyCapture->prefiltered->bind(4);
	  m_PBR->m_RenderTargetBRDFLUT->getColorTexture(0)->bind(5);
	  m_PostProcessor->SSAOOutput->bind(6);

	  Shader* ambientShader = m_MaterialLibrary->deferredAmbientShader;
	  ambientShader->use();
	  ambientShader->setInt("SSAO", m_PostProcessor->SSAO);
	  renderMesh(m_NDCPlane, ambientShader);
	}
  }

  void Renderer::renderDeferredDirLight(DirectionalLight* light) {
	Shader* dirShader = m_MaterialLibrary->deferredDirectionalShader;

	dirShader->use();
	dirShader->setVector("camPos", m_Camera->position);
	dirShader->setVector("lightDir", light->direction);
	dirShader->setVector("lightColor", light->color.normalize() * light->intensity);
	dirShader->setBool("ShadowsEnabled", Shadows);

	if (light->ShadowMapRT) {
	  dirShader->setMatrix("lightShadowViewProjection", light->lightSpaceViewProjection);
	  light->ShadowMapRT->getDepthStencilTexture()->bind(3);
	}

	renderMesh(m_NDCPlane, dirShader);
  }

  void Renderer::renderDeferredPointLight(PointLight* light) {
	Shader* pointShader = m_MaterialLibrary->deferredPointShader;

	pointShader->use();
	pointShader->setVector("camPos", m_Camera->Position);
	pointShader->setVector("lightPos", light->Position);
	pointShader->setFloat("lightRadius", light->Radius);
	pointShader->setVector("lightColor", light->color.normalize() * light->Intensity);

	math::Matrix4 model;
	math::translate(model, light->position);
	math::scale(model, math::vec3(light->radius));
	pointShader->setMatrix("model", model);

	renderMesh(m_DeferredPointMesh, pointShader);
  }

  void Renderer::renderShadowCastCommand(RenderCommand* command, const math::Matrix4& projection, const math::Matrix4& view) {
	Shader* shadowShader = m_MaterialLibrary->dirShadowShader;

	shadowShader->setMatrix("projection", projection);
	shadowShader->setMatrix("view", view);
	shadowShader->setMatrix("model", command->transform);

	renderMesh(command->mesh, shadowShader);
  }

}
