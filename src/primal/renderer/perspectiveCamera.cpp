#include "perspectiveCamera.h"
#include "primal/core/application.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

const float YAW = -90.0f;
const float PITCH =  0.0f;
const float SENSITIVITY =  1.0f;
const float ZOOM =  45.0f;

namespace primal {

  PerspectiveCamera::PerspectiveCamera(glm::vec3 position, glm::vec3 up,
									   float aspectRatio, float zoom = ZOOM,
									   float yaw = YAW, float pitch = PITCH)
   : m_position(position), m_mouseSensitivity(SENSITIVITY), m_zoom(zoom),
     m_worldUp(up), m_yaw(yaw), m_pitch(pitch) {

	PRIMAL_PROFILE_FUNCTION();

	m_projectionMatrix = glm::perspective(zoom, aspectRatio, 0.1f, 100.0f);

	recalculateVectors();
  }

  void PerspectiveCamera::setProjection(float zoom, float aspectRatio, float near, float far) {
	PRIMAL_PROFILE_FUNCTION();

	m_projectionMatrix = glm::perspective(zoom, aspectRatio, near, far);
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;

	recalculateVectors();
  }

  void PerspectiveCamera::setRotation(float mouseX, float mouseY) {
	if (m_firstMouse) {
	  m_lastX = mouseX;
	  m_lastY = mouseY;
	  m_firstMouse = false;
	}
	float xoffset = mouseX - m_lastX;
	float yoffset = m_lastY - mouseY; // reversed since y-coordinates go from bottom to top

	m_lastX = mouseX;
	m_lastY = mouseY;

	xoffset *= m_mouseSensitivity;
	yoffset *= m_mouseSensitivity;

	m_yaw += xoffset;
	m_pitch += yoffset;

	if (m_pitch > 89.0f)
	  m_pitch = 89.0f;
	if (m_pitch < -89.0f)
	  m_pitch = -89.0f;

	recalculateVectors();
  }

  void PerspectiveCamera::recalculateVectors() {
	PRIMAL_PROFILE_FUNCTION();

	glm::vec3 front;
	front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	front.y = sin(glm::radians(m_pitch));
	front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

	m_front = glm::normalize(front);
	m_right = glm::normalize(glm::cross(m_front, m_worldUp));
	m_up = glm::normalize(glm::cross(m_right, m_front));

	m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
  }

}
