#pragma once

#include <glm/glm.hpp>

namespace primal {

  class PerspectiveCamera {
	public:
	  PerspectiveCamera(glm::vec3 position, glm::vec3 up, float aspectRatio, float zoom, float yaw, float pitch);

	  void setProjection(float fov, float aspectRatio, float near, float far);

	  const glm::vec3& getPosition() const { return m_position; }
	  void setPosition(const glm::vec3& position) { m_position = position; recalculateVectors(); }

	  float getRotation() const { return m_rotation; }
	  void setRotation(float rotation) { m_rotation = rotation; recalculateVectors(); }
	  void setRotation(float mouseX, float mouseY);

	  const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
	  const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
	  const glm::mat4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }
	  const glm::vec3& getFrontVector() const { return m_front; }
	  const glm::vec3& getRightVector() const { return m_right; }
	  const glm::vec3& getUpVector() const { return m_up; }
	private:
	  void recalculateVectors();

	  glm::mat4 m_projectionMatrix;
	  glm::mat4 m_viewMatrix;
	  glm::mat4 m_viewProjectionMatrix;

	  glm::vec3 m_position;
	  glm::vec3 m_up;
	  glm::vec3 m_front;
	  glm::vec3 m_right;
	  glm::vec3 m_worldUp;
	  float m_rotation;

	  float m_yaw;
	  float m_pitch;

	  float m_mouseSensitivity;
	  float m_zoom;

	  // rotation - mouse position related
	  float m_lastX;
	  float m_lastY;
	  bool m_firstMouse = true;
  };

}
