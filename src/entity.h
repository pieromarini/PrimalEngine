#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "material.h"

namespace pm {

using EntityId = uint32_t;

struct Entity {
	EntityId id{};

	// TODO: should we store components instead?
	// glm::vec3 position;
	// glm::quat orientation;
	// glm::vec3 scale;
	glm::mat4 transform{};


	Material material;
};

}// namespace pm
