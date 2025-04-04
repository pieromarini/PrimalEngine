#include <camera.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace pm {

glm::mat4 Camera::getViewMatrix() {
	// to create a correct model view, we need to move the world in opposite
	// direction to the camera
	// so we will create the camera model matrix and invert
	glm::mat4 cameraTranslation = glm::translate(glm::mat4(1.f), position);
	glm::mat4 cameraRotation = getRotationMatrix();
	return glm::inverse(cameraTranslation * cameraRotation);
}

glm::mat4 Camera::getRotationMatrix() {
	// fairly typical FPS style camera. we join the pitch and yaw rotations into
	// the final rotation matrix
	glm::quat pitchRotation = glm::angleAxis(pitch, glm::vec3{ 1.f, 0.f, 0.f });
	glm::quat yawRotation = glm::angleAxis(yaw, glm::vec3{ 0.f, -1.f, 0.f });

	return glm::toMat4(yawRotation) * glm::toMat4(pitchRotation);
}

glm::mat4 Camera::getPerspectiveProjection() {
	return glm::perspective(glm::radians(70.f), static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 10000.f, 0.1f);
}

glm::mat4 Camera::getOrthographicProjection() {
	return glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight), -1.0f, 1.0f);
}

void Camera::onWindowResize(uint32_t width, uint32_t height) {
	windowWidth = width;
	windowHeight = height;
}

void Camera::update(float deltaTime) {
	glm::mat4 cameraRotation = getRotationMatrix();
	position += glm::vec3(cameraRotation * glm::vec4(velocity * 0.01f * deltaTime, 0.f));
}

void Camera::setMouseControlEnabled(bool flag) {
	cameraMouseControlEnabled = flag;
}

void Camera::processSDLEvent(SDL_Event& e) {
	if (e.type == SDL_EVENT_KEY_DOWN) {
		if (e.key.key == SDLK_W) {
			velocity.z = -1;
		}
		if (e.key.key == SDLK_S) {
			velocity.z = 1;
		}
		if (e.key.key == SDLK_A) {
			velocity.x = -1;
		}
		if (e.key.key == SDLK_D) {
			velocity.x = 1;
		}
	}

	if (e.type == SDL_EVENT_KEY_UP) {
		if (e.key.key == SDLK_W) {
			velocity.z = 0;
		}
		if (e.key.key == SDLK_S) {
			velocity.z = 0;
		}
		if (e.key.key == SDLK_A) {
			velocity.x = 0;
		}
		if (e.key.key == SDLK_D) {
			velocity.x = 0;
		}
	}

	if (cameraMouseControlEnabled && e.type == SDL_EVENT_MOUSE_MOTION) {
		yaw += e.motion.xrel / 200.f;
		pitch -= e.motion.yrel / 200.f;
	}
}

}// namespace pm
