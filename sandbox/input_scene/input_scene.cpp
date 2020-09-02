#include "input_scene.h"

#include "input_component.h"

using namespace primal;

void InputScene::load() {
  Entity* cameraEntity{ Entity::create("Camera") };
  auto position = math::vec3{ 0, 0, -2 };
  cameraEntity->setTransform(position, math::vec3{ 0, 0, 0 }, math::vec3{ 1, 1, 1 });

  cameraEntity->addComponent<CameraComponent>(position);
  cameraEntity->addComponent<InputTestComponent>();
}
