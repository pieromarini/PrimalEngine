#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

namespace pm {

constexpr uint32_t VOXEL_CHUNK_SIZE = 4;
constexpr uint32_t VOXEL_CHUNK_COUNT = VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE;

constexpr float VOXEL_SIZE = 1.0f;

struct Voxel {
	uint32_t id;
	float x, y, z;
	glm::vec4 color;
};

struct VoxelVertex {
	glm::vec3 position;
	float uv_x{ 0.0f };
	glm::vec3 normal;
	float uv_y{ 0.0f };
	glm::vec4 color;
	glm::vec4 padding{ 0.0f };
};

struct VoxelChunk {
	float x, y, z;
	std::vector<Voxel> voxels;
	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
	glm::mat4 transform;
};

struct VoxelTerrain {
	std::vector<VoxelChunk> chunks;
};

void generateTerrainGeometry(VoxelTerrain& terrain, std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices);
VoxelTerrain generateTerrain();

}// namespace pm
