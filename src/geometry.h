#pragma once

#include "material.h"
#include "vk_types.h"
#include <cstdint>

namespace pm {

struct MeshPrimitive {
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t indexCount;
	MaterialIndex materialIndex;
	MaterialPass passType;
};

struct Mesh {
	std::string name;
	glm::mat4 transform;
	std::vector<MeshPrimitive> primitives;
};

}// namespace pm
