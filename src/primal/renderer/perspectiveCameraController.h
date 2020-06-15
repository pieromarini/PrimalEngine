#pragma once

#include "perspectiveCamera.h"
#include "primal/core/timestep.h"

#include "primal/events/mouseEvent.h"
#include "primal/events/applicationEvent.h"


namespace primal {

  class PerspectiveCameraController {
	public:
	  PerspectiveCameraController(float aspectRatio, bool rotation = false);

	  void onUpdate(Timestep ts);
	  void onEvent(Event& e);

	  [[nodiscard]] PerspectiveCamera& getCamera() { return m_camera; }
	  [[nodiscard]] const PerspectiveCamera& getCamera() const { return m_camera; }

	  [[nodiscard]] float getZoomLevel() const { return m_zoomLevel; }
	  void setZoomLevel(float level) { m_zoomLevel = level; }

	private:
	  bool onMouseScrolled(MouseScrolledEvent& e);
	  bool onWindowResized(WindowResizeEvent& e);
	  bool onMouseMoved(MouseMovedEvent& e);

	  float m_aspectRatio;
	  float m_zoomLevel = 1.0f;
	  PerspectiveCamera m_camera;

	  bool m_rotation;

	  glm::vec3 m_cameraPosition = { 0.0f, 0.0f, 0.0f };
	  float m_cameraRotation = 0.0f; // degrees, anti-clockwise direction
	  float m_cameraTranslationSpeed = 30.0f;
	  float m_cameraRotationSpeed = 30.0f;
  };
}
