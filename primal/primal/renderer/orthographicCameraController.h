#pragma once

#include "orthographicCamera.h"
#include "../core/timestep.h"

#include "../events/mouseEvent.h"
#include "../events/applicationEvent.h"


namespace primal {

  class OrthographicCameraController {
	public:
	  OrthographicCameraController(float aspectRatio, bool rotation = false);

	  void onUpdate(Timestep ts);
	  void onEvent(Event& e);

	  OrthographicCamera& getCamera() { return m_camera; }
	  const OrthographicCamera& getCamera() const { return m_camera; }

	  float getZoomLevel() const { return m_zoomLevel; }
	  void setZoomLevel(float level) { m_zoomLevel = level; }

	private:
	  bool onMouseScrolled(MouseScrolledEvent& e);
	  bool onWindowResized(WindowResizeEvent& e);

	  float m_aspectRatio;
	  float m_zoomLevel = 1.0f;
	  OrthographicCamera m_camera;

	  bool m_rotation;

	  glm::vec3 m_cameraPosition = { 0.0f, 0.0f, 0.0f };
	  float m_cameraRotation = 0.0f; // degrees, anti-clockwise direction
	  float m_cameraTranslationSpeed = 5.0f, m_cameraRotationSpeed = 180.0f;
  };
}
