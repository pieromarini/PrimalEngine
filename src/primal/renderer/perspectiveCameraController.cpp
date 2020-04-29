#include "GLFW/glfw3.h"
#include "primal/core/core.h"
#include "primal/core/application.h"

#include "perspectiveCameraController.h"

namespace primal {

  PerspectiveCameraController::PerspectiveCameraController(float aspectRatio, bool rotation)
	: m_aspectRatio(aspectRatio), m_camera(m_cameraPosition, {0.0f, 1.0f, 0.0f}, m_aspectRatio, m_zoomLevel, -90.0f, 0.0f), m_rotation(rotation) { }

  void PerspectiveCameraController::onUpdate(Timestep ts) {
	PRIMAL_PROFILE_FUNCTION();

	if (Input::isKeyPressed(PRIMAL_KEY_A)) {
	  m_cameraPosition -= m_camera.getRightVector() * m_cameraTranslationSpeed * ts.getSeconds();
	} else if (Input::isKeyPressed(PRIMAL_KEY_D)) {
	  m_cameraPosition += m_camera.getRightVector() * m_cameraTranslationSpeed * ts.getSeconds();
	}

	if (Input::isKeyPressed(PRIMAL_KEY_W)) {
	  m_cameraPosition += m_camera.getFrontVector() * m_cameraTranslationSpeed * ts.getSeconds();
	} else if (Input::isKeyPressed(PRIMAL_KEY_S)) {
	  m_cameraPosition -= m_camera.getFrontVector() * m_cameraTranslationSpeed * ts.getSeconds();
	}

	if (Input::isKeyPressed(PRIMAL_KEY_Q)) {
	  m_cameraPosition += m_camera.getUpVector() * m_cameraTranslationSpeed * ts.getSeconds();
	} else if (Input::isKeyPressed(PRIMAL_KEY_E)) {
	  m_cameraPosition -= m_camera.getUpVector() * m_cameraTranslationSpeed * ts.getSeconds();
	}

	if (Input::isKeyPressed(PRIMAL_KEY_P)) {
	  Application::get().getWindow().toggleCursor();
	}

	m_camera.setPosition(m_cameraPosition);

	m_cameraTranslationSpeed = m_zoomLevel;
  }

  void PerspectiveCameraController::onEvent(Event& e) {
	PRIMAL_PROFILE_FUNCTION();

	EventDispatcher dispatcher(e);
	// TODO: Use lambdas here instead of bind.
	dispatcher.dispatch<MouseScrolledEvent>(PRIMAL_BIND_EVENT_FN(PerspectiveCameraController::onMouseScrolled));
	dispatcher.dispatch<WindowResizeEvent>(PRIMAL_BIND_EVENT_FN(PerspectiveCameraController::onWindowResized));
	dispatcher.dispatch<MouseMovedEvent>(PRIMAL_BIND_EVENT_FN(PerspectiveCameraController::onMouseMoved));
  }

  bool PerspectiveCameraController::onMouseScrolled(MouseScrolledEvent& e) {
	PRIMAL_PROFILE_FUNCTION();

	m_zoomLevel -= e.getYOffset() * 0.25f;
	m_zoomLevel = std::max(m_zoomLevel, 0.25f);
	m_camera.setProjection(m_zoomLevel, m_aspectRatio, 0.1f, 100.0f);
	return false;
  }

  bool PerspectiveCameraController::onMouseMoved(MouseMovedEvent& e) {
	PRIMAL_PROFILE_FUNCTION();

	// NOTE: TEMP. Stop rotating the camera while on free mouse mode.
	if (Application::get().getWindow().getCursorMode() == GLFW_CURSOR_NORMAL) {
	  return false;
	}

	m_camera.setRotation(e.getX(), e.getY());
	return false;
  }

  bool PerspectiveCameraController::onWindowResized(WindowResizeEvent& e) {
	PRIMAL_PROFILE_FUNCTION();

	m_aspectRatio = (float)e.getWidth() / (float)e.getHeight();
	m_camera.setProjection(m_zoomLevel, m_aspectRatio, 0.1f, 100.0f);
	return false;
  }

}
