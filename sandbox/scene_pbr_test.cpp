#include "scene_pbr_test.h"

#include "shading/material.h"
#include "resources/resources.h"
#include "scene/scene.h"
#include "scene/scene_node.h"
#include "camera/camera.h"

ScenePbrTest::ScenePbrTest(primal::renderer::Renderer* renderer, primal::Camera* camera) {
  m_Renderer = renderer;
  m_Camera = camera;

  m_PbrShader = primal::renderer::Resources::loadShader("pbr", "res/shaders/pbr.vs", "res/shaders/pbr.frag");
  m_PBRMaterial.setShader(m_PbrShader);
}

ScenePbrTest::~ScenePbrTest() {
}

void ScenePbrTest::init() {
  // TODO(Joey): pre-compute irradiance/prefilter/integrate map  for testing
}

void ScenePbrTest::update(float dt) {
  m_Camera->update(dt);
}

void ScenePbrTest::render() {
}
