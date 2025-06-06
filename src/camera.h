#pragma once

#include <SDL3/SDL_events.h>
#include "core/math/math.h"

namespace pm {

class Camera {
public:
	// NOTE: Don't allow constructing a camera without window size
	Camera() = delete;
	Camera(uint32_t windowW, uint32_t windowH) : windowWidth{ windowW }, windowHeight{ windowH } {}

	mat4 getViewMatrix();
	mat4 getRotationMatrix();

	mat4 getPerspectiveProjection();
	mat4 getOrthographicProjection();

	void processSDLEvent(SDL_Event& e);

	void update(float deltaTime);

	void onWindowResize(uint32_t width, uint32_t height);

	void setMouseControlEnabled(bool flag);

	uint32_t windowWidth, windowHeight;
	vec3 velocity{};
	vec3 position{};
	float pitch{ 0.f };
	float yaw{ 0.f };

	bool cameraMouseControlEnabled{ true };
};

}// namespace pm
