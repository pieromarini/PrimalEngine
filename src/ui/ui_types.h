#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace pm {

struct UIVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 color;
	float uv_y;
};

struct UIElement {
	glm::vec2 position;
	glm::vec2 scale;
	float rotation;
	float width;
	float height;
	std::vector<UIVertex> vertices{};
	std::vector<uint32_t> indices{};
};

}// namespace pm
