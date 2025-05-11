#include "voxel.h"
#include "glm/ext/matrix_transform.hpp"
#include <format>
#include <iostream>

namespace pm {

uint32_t packPosition(uint8_t x, uint8_t y, uint8_t z) {
	return (z << 10) | (y << 5) | x;
}

void unpackPosition(uint32_t packed, uint8_t& x, uint8_t& y, uint8_t& z) {
	x = packed & 0x1F;
	y = (packed >> 5) & 0x1F;
	z = (packed >> 10) & 0x1F;
}

void generateTerrainGeometry(VoxelTerrain& terrain, std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices) {
	// Create a 3D lookup grid to check for adjacent voxels
	std::unordered_map<int64_t, bool> voxelMap;

	// Fill the voxel lookup map
	for (auto& chunk : terrain.chunks) {
		auto chunkOffset = glm::vec3(chunk.transform[3]);// Extract translation from transform matrix

		for (auto& voxel : chunk.voxels) {
			// Calculate world position
			int32_t worldX = static_cast<int32_t>(chunkOffset.x) + voxel.x;
			int32_t worldY = static_cast<int32_t>(chunkOffset.y) + voxel.y;
			int32_t worldZ = static_cast<int32_t>(chunkOffset.z) + voxel.z;

			// std::cout << std::format("{} {} {}\n", worldX, worldY, worldZ);

			// Create a unique key for this position
			int64_t key = (static_cast<int64_t>(worldX) << 40) | (static_cast<int64_t>(worldY) << 20) | static_cast<int64_t>(worldZ);

			voxelMap.try_emplace(key, true);
		}
	}

	// Helper function to check if a voxel exists at a position
	auto voxelExists = [&voxelMap](int32_t x, int32_t y, int32_t z) -> bool {
		int64_t key = (static_cast<int64_t>(x) << 40) | (static_cast<int64_t>(y) << 20) | static_cast<int64_t>(z);
		// std::cout << std::format("{} {} {} | {} -> {}\n", x, y, z, key, r);
		return voxelMap.find(key) != voxelMap.end();
	};

	for (auto& chunk : terrain.chunks) {
		auto chunkOffset = glm::vec3(chunk.transform[3]);// Extract translation

		chunk.firstIndex = indices.size();
		chunk.vertexOffset = static_cast<int32_t>(vertices.size());

		size_t indicesBeforeChunk = indices.size();

		auto baseIndex = 0;

		for (auto& voxel : chunk.voxels) {
			uint8_t size = VOXEL_SIZE;

			// Calculate world position
			int32_t worldX = static_cast<int32_t>(chunkOffset.x) + voxel.x;
			int32_t worldY = static_cast<int32_t>(chunkOffset.y) + voxel.y;
			int32_t worldZ = static_cast<int32_t>(chunkOffset.z) + voxel.z;

			// Vertex indices for the current voxel
			auto color = voxel.color;

			// Only generate faces that are not obscured by adjacent voxels

			// Front face (Z+) red
			if (!voxelExists(worldX, worldY, worldZ + 1)) {
				if (DEBUG_VOXEL_COLORS) {
					color = { 1.0f, 0.0f, 0.0f, 1.0f };
				}
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x, voxel.y, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x + size, voxel.y, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z + size), .color = color  });
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x, voxel.y + size, voxel.z + size), .color = color  });

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 1);
				indices.push_back(baseIndex + 2);

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 3);

				baseIndex += 4;
			}

			// Back face (Z-) blue
			if (!voxelExists(worldX, worldY, worldZ - 1)) {
				if (DEBUG_VOXEL_COLORS) {
					color = { 0.0f, 0.0f, 1.0f, 1.0f };
				}
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x, voxel.y, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x, voxel.y + size, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x + size, voxel.y, voxel.z), .color = color });

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 1);
				indices.push_back(baseIndex + 2);

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 3);

				baseIndex += 4;
			}

			// Right face (X+) green
			if (!voxelExists(worldX + 1, worldY, worldZ)) {
				if (DEBUG_VOXEL_COLORS) {
					color = { 0.0f, 1.0f, 0.0f, 1.0f };
				}
				vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y, voxel.z + size), .color = color });

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 1);
				indices.push_back(baseIndex + 2);

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 3);

				baseIndex += 4;
			}

			// Left face (X-) cyan
			if (!voxelExists(worldX - 1, worldY, worldZ)) {
				if (DEBUG_VOXEL_COLORS) {
					color = { 0.0f, 1.0f, 1.0f, 1.0f };
				}
				vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x, voxel.y, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x, voxel.y, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x, voxel.y + size, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x, voxel.y + size, voxel.z), .color = color });

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 1);
				indices.push_back(baseIndex + 2);

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 3);

				baseIndex += 4;
			}

			// Top face (Y+) yellow
			if (!voxelExists(worldX, worldY + 1, worldZ)) {
				if (DEBUG_VOXEL_COLORS) {
					color = { 1.0f, 1.0f, 0.0f, 1.0f };
				}
				vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x, voxel.y + size, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x, voxel.y + size, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z), .color = color });

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 1);
				indices.push_back(baseIndex + 2);

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 3);

				baseIndex += 4;
			}

			// Bottom face (Y-) white
			if (!voxelExists(worldX, worldY - 1, worldZ)) {
				if (DEBUG_VOXEL_COLORS) {
					color = { 1.0f, 1.0f, 1.0f, 1.0f };
				}
				vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x, voxel.y, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y, voxel.z), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x, voxel.y, voxel.z + size), .color = color });

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 1);
				indices.push_back(baseIndex + 2);

				indices.push_back(baseIndex);
				indices.push_back(baseIndex + 2);
				indices.push_back(baseIndex + 3);

				baseIndex += 4;
			}
		}

		// Update the index count for this chunk based on actual generated faces
		chunk.indexCount = indices.size() - indicesBeforeChunk;
	}
}

VoxelTerrain generateTerrain() {
	VoxelTerrain terrain{};

	constexpr uint32_t TERRAIN_DIMENSION = 20;

	for (uint32_t chunkZ = 0; chunkZ < TERRAIN_DIMENSION; ++chunkZ) {
		for (uint32_t chunkX = 0; chunkX < TERRAIN_DIMENSION; ++chunkX) {
			VoxelChunk voxelChunk{};
			voxelChunk.voxels.resize(VOXEL_CHUNK_COUNT);

			for (uint32_t y = 0; y < VOXEL_CHUNK_SIZE; ++y) {
				for (uint32_t z = 0; z < VOXEL_CHUNK_SIZE; ++z) {
					for (uint32_t x = 0; x < VOXEL_CHUNK_SIZE; ++x) {
						uint32_t voxelIndex = x + (z * VOXEL_CHUNK_SIZE) + (y * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE);
						voxelChunk.voxels[voxelIndex] = {
							.id = voxelIndex,
							.x = (uint8_t)x,
							.y = (uint8_t)y,
							.z = (uint8_t)z,
							.color = { 0.0f, (float)(chunkZ + 1) / TERRAIN_DIMENSION, (float)(chunkX + 1) / TERRAIN_DIMENSION, 1.0f }
						};
					}
				}
			}

			voxelChunk.transform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ chunkX * VOXEL_CHUNK_SIZE, 0.0f, chunkZ * VOXEL_CHUNK_SIZE });

			terrain.chunks.push_back(voxelChunk);
		}
	}

	return terrain;
}

}// namespace pm
