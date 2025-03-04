#pragma once

#include <SDL3/SDL_events.h>
#include <vk_types.h>

namespace pm {

class Camera {
public:
	// NOTE: Don't allow constructing a camera without window size
	Camera() = delete;
	Camera(uint32_t windowW, uint32_t windowH) : windowWidth{ windowW }, windowHeight{ windowH } {}

	glm::mat4 getViewMatrix();
	glm::mat4 getRotationMatrix();

	glm::mat4 getPerspectiveProjection();
	glm::mat4 getOrthographicProjection();

	void processSDLEvent(SDL_Event& e);

	void update(float deltaTime);

	void onWindowResize(uint32_t width, uint32_t height);

	uint32_t windowWidth, windowHeight;
	glm::vec3 velocity{};
	glm::vec3 position{};
	float pitch{ 0.f };
	float yaw{ 0.f };
};

}// namespace pm
