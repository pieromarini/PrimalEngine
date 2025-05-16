#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <vector>

namespace pm {

constexpr uint32_t TERRAIN_DIMENSION = 16;

constexpr uint32_t VOXEL_CHUNK_SIZE = 16;
constexpr uint32_t VOXEL_CHUNK_SIZE_Y = 64;
constexpr uint32_t VOXEL_CHUNK_COUNT = VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE_Y;

constexpr uint8_t VOXEL_SIZE = 1;

constexpr bool DEBUG_VOXEL_COLORS = false;
constexpr bool DEBUG_CHUNK_COLORS = false;

struct Voxel {
	uint32_t id;
	uint8_t x, y, z;
	glm::vec4 color;
	bool empty { true };
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

struct alignas(16) VoxelGrid {
	glm::vec3 minBound;
	uint32_t numVoxelsX;

	glm::vec3 maxBound;
	uint32_t numVoxelsY;

	glm::vec3 gridSize;
	uint32_t numVoxelsZ;
};

struct VoxelTerrain {
	std::vector<VoxelChunk> chunks;
	std::vector<uint32_t> voxelData;
	uint64_t voxelCount{ 0 };
	VoxelGrid grid;
};

uint32_t getVoxelIndex(uint32_t localX, uint32_t y, uint32_t localZ, uint32_t chunkX, uint32_t chunkZ);

void generateTerrainGeometry(VoxelTerrain& terrain, std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices);
VoxelTerrain generateTerrain();

// helpers
uint32_t packPosition(uint8_t x, uint8_t y, uint8_t z);
void unpackPosition(uint32_t packed, uint8_t& x, uint8_t& y, uint8_t& z);

}// namespace pm
