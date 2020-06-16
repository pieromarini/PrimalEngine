#pragma once

#include <glm/glm.hpp>

namespace primal {

  class OrthographicCamera {
	public:
	  OrthographicCamera(float left, float right, float bottom, float top);

	  void setProjection(float left, float right, float bottom, float top);

	  [[nodiscard]] const glm::vec3& getPosition() const { return m_position; }
	  void setPosition(const glm::vec3& position) { m_position = position; recalculateViewMatrix(); }

	  [[nodiscard]] float getRotation() const { return m_rotation; }
	  void setRotation(float rotation) { m_rotation = rotation; recalculateViewMatrix(); }

	  [[nodiscard]] const glm::mat4& getProjectionMatrix() const { return m_projectionMatrix; }
	  [[nodiscard]] const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
	  [[nodiscard]] const glm::mat4& getViewProjectionMatrix() const { return m_viewProjectionMatrix; }
	private:
	  void recalculateViewMatrix();

	  glm::mat4 m_projectionMatrix;
	  glm::mat4 m_viewMatrix;
	  glm::mat4 m_viewProjectionMatrix;

	  glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
	  float m_rotation = 0.0f;
  };

}
