#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

namespace pm {

constexpr uint32_t VOXEL_CHUNK_SIZE = 16;
constexpr uint32_t VOXEL_CHUNK_COUNT = VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE;

constexpr uint8_t VOXEL_SIZE = 1;

struct Voxel {
	uint32_t id;
	uint8_t x, y, z;
	glm::vec4 color;
};
 
 struct VoxelVertex {
 	glm::vec3 normal{ 0.0f };
 	uint32_t data;
	glm::vec4 color;
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

// helpers
uint32_t packPosition(uint8_t x, uint8_t y, uint8_t z);
void unpackPosition(uint32_t packed, uint8_t& x, uint8_t& y, uint8_t& z);

}// namespace pm
