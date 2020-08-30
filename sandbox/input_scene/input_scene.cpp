#include "input_scene.h"

#include "input_component.h"

using namespace primal;

void InputScene::load() {
  Entity* cameraEntity{ Entity::create("Camera") };
  cameraEntity->setTransform(math::vec3{ 0, 5, 10 }, math::vec3{ -15, 0, 0 }, math::vec3{ 1, 1, 1 });

  cameraEntity->addComponent<CameraComponent>();
  cameraEntity->addComponent<InputTestComponent>();
}
