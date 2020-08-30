#include <iostream>

#include "application.h"
#include "core/math/math.h"
#include "tools/log.h"
#include "primal.h"

#include "scene_pbr_test.h"

#include "utility/random/random.h"

#include <GLFW/glfw3.h>

void framebufferSizeFunc(GLFWwindow* window, int width, int height);
void keyFunc(GLFWwindow* window, int key, int scancode, int action, int mods);
void mousePosFunc(GLFWwindow* window, double xpos, double ypos);
void mouseButtonFunc(GLFWwindow*, int button, int action, int mods);
void mouseScrollFunc(GLFWwindow*, double xoffset, double yoffset);

primal::renderer::Renderer* renderer;
primal::FlyCamera camera(math::vec3(0.0f, 1.0f, 0.0f), math::vec3(0.0f, 0.0f, -1.0f));
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;
bool keysPressed[1024];
bool keysActive[1024];
bool wireframe = false;
bool renderGUI = false;

int main(int argc, char* argv[]) {

  primal::Application app;
  app.start();

  // basic shapes
  primal::Plane plane(16, 16);
  primal::Sphere sphere(64, 64);
  primal::Sphere tSphere(256, 256);
  primal::Torus torus(2.0f, 0.4f, 32, 32);
  primal::Cube cube;

  // material setup
  primal::Material* matPbr = renderer->CreateMaterial();
  primal::Shader* plasmaOrbShader = primal::Resources::LoadShader("plasma orb", "shaders/custom/plasma_orb.vs", "shaders/custom/plasma_orb.fs");
  primal::Material* matPlasmaOrb = renderer->CreateCustomMaterial(plasmaOrbShader);
  matPlasmaOrb->Cull = false;
  matPlasmaOrb->Blend = true;
  matPlasmaOrb->BlendSrc = GL_ONE;
  matPlasmaOrb->BlendDst = GL_ONE;
  matPlasmaOrb->SetTexture("TexPerllin", primal::Resources::LoadTexture("perlin noise", "textures/perlin.png"), 0);
  matPlasmaOrb->SetFloat("Strength", 1.5f);
  matPlasmaOrb->SetFloat("Speed", 0.083f);
  // configure camera
  camera.SetPerspective(math::Deg2Rad(60.0f), renderer->GetRenderSize().x / renderer->GetRenderSize().y, 0.1f, 100.0f);

  // scene setup
  primal::SceneNode* mainTorus = primal::Scene::MakeSceneNode(&torus, matPbr);
  primal::SceneNode* secondTorus = primal::Scene::MakeSceneNode(&torus, matPbr);
  primal::SceneNode* thirdTorus = primal::Scene::MakeSceneNode(&torus, matPbr);
  primal::SceneNode* plasmaOrb = primal::Scene::MakeSceneNode(&tSphere, matPlasmaOrb);

  mainTorus->AddChild(secondTorus);
  secondTorus->AddChild(thirdTorus);

  mainTorus->SetScale(1.0f);
  mainTorus->SetPosition(math::vec3(-4.4f, 3.46f, -0.3));
  secondTorus->SetScale(0.65f);
  secondTorus->SetRotation(math::vec4(0.0, 1.0, 0.0, math::Deg2Rad(90.0)));
  thirdTorus->SetScale(0.65f);

  plasmaOrb->SetPosition(math::vec3(-4.0f, 4.0f, 0.25f));
  plasmaOrb->SetScale(0.6f);

  // - background
  primal::Background* background = new primal::Background;
  primal::PBRCapture* pbrEnv = renderer->GetSkypCature();
  background->SetCubemap(pbrEnv->Prefiltered);
  float lodLevel = 1.5f;
  background->material->SetFloat("lodLevel", lodLevel);

  // post processing
  primal::Shader* postProcessing1 = primal::Resources::LoadShader("postprocessing1", "shaders/screen_quad.vs", "shaders/custom_post_1.fs");
  primal::Shader* postProcessing2 = primal::Resources::LoadShader("postprocessing2", "shaders/screen_quad.vs", "shaders/custom_post_2.fs");
  primal::Material* customPostProcessing1 = renderer->CreatePostProcessingMaterial(postProcessing1);
  primal::Material* customPostProcessing2 = renderer->CreatePostProcessingMaterial(postProcessing2);

  // mesh
  primal::SceneNode* sponza = primal::Resources::LoadMesh(renderer, "sponza", "meshes/sponza/sponza.obj");
  sponza->SetPosition(math::vec3(0.0, -1.0, 0.0));
  sponza->SetScale(0.01f);

  // lighting
  primal::DirectionalLight dirLight;
  dirLight.Direction = math::vec3(0.2f, -1.0f, 0.25f);
  dirLight.Color = math::vec3(1.0f, 0.89f, 0.7f);
  dirLight.Intensity = 50.0f;
  renderer->AddLight(&dirLight);

  std::vector<primal::PointLight> torchLights;
  {
	primal::PointLight torch;
	torch.Radius = 2.5;
	torch.Color = math::vec3(1.0f, 0.3f, 0.05f);
	torch.Intensity = 50.0f;
	torch.RenderMesh = true;

	torch.Position = math::vec3(4.85f, 0.7f, 1.43f);
	torchLights.push_back(torch);
	torch.Position = math::vec3(4.85f, 0.7f, -2.2f);
	torchLights.push_back(torch);
	torch.Position = math::vec3(-6.19f, 0.7f, 1.43f);
	torchLights.push_back(torch);
	torch.Position = math::vec3(-6.19f, 0.7f, -2.2f);
	torchLights.push_back(torch);
	renderer->AddLight(&torchLights[0]);
	renderer->AddLight(&torchLights[1]);
	renderer->AddLight(&torchLights[2]);
	renderer->AddLight(&torchLights[3]);
  }

  std::vector<primal::PointLight> randomLights;
  std::vector<math::vec3> randomLightStartPositions;
  {
	for (int i = 0; i < 100; ++i) {
	  primal::PointLight light;
	  light.Radius = 1.0 + Random::Uniliteral() * 3.0;
	  light.Intensity = 10.0 + Random::Uniliteral() * 1000.0;
	  light.Color = math::vec3(Random::Uniliteral(), Random::Uniliteral(), Random::Uniliteral());
	  light.RenderMesh = true;
	  randomLights.push_back(light);
	  randomLightStartPositions.push_back(math::vec3(Random::Biliteral() * 12.0f, Random::Uniliteral() * 5.0f, Random::Biliteral() * 6.0f));
	}
	for (int i = 0; i < randomLights.size(); ++i) {
	  // uncomment for deferred lighting madness
	  //renderer->AddLight(&randomLights[i]);
	}
  }

  // bake irradiance GI (with grid placement of probes)
  {
	// bottom floor - center
	renderer->AddIrradianceProbe(math::vec3(0.0f, 0.5f, -0.5f), 3.25);
	renderer->AddIrradianceProbe(math::vec3(3.0f, 0.5f, -0.5f), 3.25);
	renderer->AddIrradianceProbe(math::vec3(6.0f, 0.5f, -0.5f), 3.25);
	renderer->AddIrradianceProbe(math::vec3(8.5f, 0.5f, -0.5f), 3.25);
	renderer->AddIrradianceProbe(math::vec3(11.4f, 0.5f, -0.5f), 4.25);
	renderer->AddIrradianceProbe(math::vec3(-3.0f, 0.5f, -0.5f), 3.25);
	renderer->AddIrradianceProbe(math::vec3(-6.2f, 0.5f, -0.5f), 3.25);
	renderer->AddIrradianceProbe(math::vec3(-9.5f, 0.5f, -0.5f), 3.25);
	renderer->AddIrradianceProbe(math::vec3(-12.1f, 0.5f, -0.5f), 4.25);
	// bottom floor - left wing
	renderer->AddIrradianceProbe(math::vec3(0.0f, 0.5f, 4.0f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(4.0f, 0.5f, 4.0f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(8.0f, 0.5f, 4.0f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(12.0f, 0.5f, 4.0f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-4.0f, 0.5f, 4.0f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-8.0f, 0.5f, 4.0f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-12.0f, 0.5f, 4.0f), 4.0);
	// bottom floor - right wing
	renderer->AddIrradianceProbe(math::vec3(0.0f, 0.5f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(4.0f, 0.5f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(8.0f, 0.5f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(12.0f, 0.5f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-4.0f, 0.5f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-8.0f, 0.5f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-12.0f, 0.5f, -4.5f), 4.0);
	// 1st floor - center wing
	renderer->AddIrradianceProbe(math::vec3(0.0f, 5.0f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(4.0f, 5.0f, -0.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(8.0f, 5.0f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(12.0f, 5.0f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(-4.0f, 5.0f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(-8.0f, 5.0f, -0.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-12.0f, 5.0f, -0.5f), 4.5);
	// 1st floor - left wing
	renderer->AddIrradianceProbe(math::vec3(0.0f, 5.0f, 4.0), 4.0);
	renderer->AddIrradianceProbe(math::vec3(4.0f, 5.0f, 4.0), 4.0);
	renderer->AddIrradianceProbe(math::vec3(8.0f, 5.0f, 4.0), 4.0);
	renderer->AddIrradianceProbe(math::vec3(12.0f, 5.0f, 4.0), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-4.0f, 5.0f, 4.0), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-8.0f, 5.0f, 4.0), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-11.5f, 5.0f, 4.0), 4.0);
	// 1st floor - right wing
	renderer->AddIrradianceProbe(math::vec3(0.0f, 5.0f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(4.0f, 5.0f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(8.0f, 5.0f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(12.0f, 5.0f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-4.0f, 5.0f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-8.0f, 5.0f, -4.5f), 4.0);
	renderer->AddIrradianceProbe(math::vec3(-11.5f, 5.0f, -4.5f), 4.0);
	// 2nd floor - center wing
	renderer->AddIrradianceProbe(math::vec3(0.0f, 9.5f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(4.0f, 9.5f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(8.0f, 9.5f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(12.0f, 9.5f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(-4.0f, 9.5f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(-8.0f, 9.5f, -0.5f), 4.5);
	renderer->AddIrradianceProbe(math::vec3(-11.5f, 9.5f, -0.5f), 4.5);

	// bake before rendering
	renderer->BakeProbes();
  }

  return 0;
}
