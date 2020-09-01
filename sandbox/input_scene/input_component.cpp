#include "input_component.h"

using namespace primal;

void InputTestComponent::onEnable() {
  handleA = Input::registerKeyPressCallback(KeyCode::A, []() {
	PRIMAL_TRACE("A pressed");
  });

  handleB = Input::registerKeyReleaseCallback(KeyCode::A, []() {
	PRIMAL_TRACE("A released");
  });

  handleC = Input::registerMousePressCallback(MouseButton::LEFT, [&]() {
	PRIMAL_TRACE("Left pressed at: ", Input::getMousePosition());

    Input::unregisterMousePressCallback(MouseButton::LEFT, handleC);

    handleC = -1;
  });
}

void InputTestComponent::onDisable() {
  Input::unregisterKeyPressCallback(KeyCode::A, handleA);
  Input::unregisterKeyReleaseCallback(KeyCode::A, handleB);

  if (handleC >= 0)
    Input::unregisterMousePressCallback(MouseButton::LEFT, handleC);
}

void InputTestComponent::update() {

  if (Input::isKeyPressed(KeyCode::B)) {
	PRIMAL_TRACE("Pressed B");
  }

  if (Input::isMouseButtonPressed(MouseButton::MIDDLE)) {
	PRIMAL_TRACE("Pressed Middle Mouse Button");
  }

}
