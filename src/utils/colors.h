#pragma once
#include <glm/glm.hpp>

inline glm::vec3 hsvToRgb(float h, float s, float v) {
	float c = v * s;
	float x = c * static_cast<float>(1.0f - std::fabs(fmod(h / 60.0f, 2) - 1.0f));
	float m = v - c;

	glm::vec3 rgb;

	if (h < 60.0f)
		rgb = glm::vec3(c, x, 0.0f);
	else if (h < 120.0f)
		rgb = glm::vec3(x, c, 0.0f);
	else if (h < 180.0f)
		rgb = glm::vec3(0.0f, c, x);
	else if (h < 240.0f)
		rgb = glm::vec3(0.0f, x, c);
	else if (h < 300.0f)
		rgb = glm::vec3(x, 0.0f, c);
	else
		rgb = glm::vec3(c, 0.0f, x);

	return rgb + glm::vec3(m);
}
