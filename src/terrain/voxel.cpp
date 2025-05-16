#include "voxel.h"
#include "glm/ext/matrix_transform.hpp"
#include "math/noise.h"
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
	std::unordered_map<int64_t, bool> voxelMap;

	// Fill the voxel lookup map
	for (auto& chunk : terrain.chunks) {
		// Extract translation from transform matrix
		auto chunkOffset = glm::vec3(chunk.transform[3]);

		for (auto& voxel : chunk.voxels) {
			// Calculate world position
			int32_t worldX = static_cast<int32_t>(chunkOffset.x) + voxel.x;
			int32_t worldY = static_cast<int32_t>(chunkOffset.y) + voxel.y;
			int32_t worldZ = static_cast<int32_t>(chunkOffset.z) + voxel.z;

			// Create a unique key for this position
			int64_t key = (static_cast<int64_t>(worldX) << 40) | (static_cast<int64_t>(worldY) << 20) | static_cast<int64_t>(worldZ);

			voxelMap.try_emplace(key, !voxel.empty);
		}
	}

	// Helper function to check if a voxel exists at a position
	auto voxelExists = [&voxelMap](int32_t x, int32_t y, int32_t z) -> bool {
		int64_t key = (static_cast<int64_t>(x) << 40) | (static_cast<int64_t>(y) << 20) | static_cast<int64_t>(z);
		auto voxel = voxelMap.find(key);
		return voxel != voxelMap.end() ? voxel->second : false;
	};

	for (auto& chunk : terrain.chunks) {
		auto chunkOffset = glm::vec3(chunk.transform[3]);// Extract translation

		chunk.firstIndex = indices.size();
		chunk.vertexOffset = static_cast<int32_t>(vertices.size());

		size_t indicesBeforeChunk = indices.size();

		auto baseIndex = 0;

		for (auto& voxel : chunk.voxels) {
			if (voxel.empty) {
				continue;
			}
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
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z + size), .color = color });
				vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x, voxel.y + size, voxel.z + size), .color = color });

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

uint32_t getVoxelIndex(uint32_t localX, uint32_t y, uint32_t localZ, uint32_t chunkX, uint32_t chunkZ) {
	uint32_t globalX = localX + (chunkX * VOXEL_CHUNK_SIZE);
	uint32_t globalZ = localZ + (chunkZ * VOXEL_CHUNK_SIZE);

	uint32_t totalXSize = TERRAIN_DIMENSION * VOXEL_CHUNK_SIZE;
	uint32_t totalZSize = TERRAIN_DIMENSION * VOXEL_CHUNK_SIZE;

	return globalX + (globalZ * totalXSize) + (y * totalXSize * totalZSize);
}

VoxelTerrain generateTerrain() {
	VoxelTerrain terrain{};

	// Create noise generators with different seeds
	const siv::PerlinNoise::seed_type seed = 728492752u;
	const siv::PerlinNoise perlin{ seed };

	// Terrain generation parameters
	constexpr float HEIGHT_SCALE = 35.0f;
	constexpr float NOISE_SCALE = 0.03f;

	// TODO(piero): handle multiple chunks
	terrain.grid.minBound = glm::vec3(0.0f, 0.0f, 0.0f);
	terrain.grid.maxBound = glm::vec3(VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION, VOXEL_CHUNK_SIZE_Y, VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION);
	terrain.grid.gridSize = terrain.grid.maxBound - terrain.grid.minBound;
	terrain.grid.numVoxelsX = VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION;
	terrain.grid.numVoxelsY = VOXEL_CHUNK_SIZE_Y;
	terrain.grid.numVoxelsZ = VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION;

	terrain.voxelData.resize(VOXEL_CHUNK_COUNT * TERRAIN_DIMENSION * TERRAIN_DIMENSION);

	for (uint32_t chunkZ = 0; chunkZ < TERRAIN_DIMENSION; ++chunkZ) {
		for (uint32_t chunkX = 0; chunkX < TERRAIN_DIMENSION; ++chunkX) {
			VoxelChunk voxelChunk{};
			voxelChunk.voxels.resize(VOXEL_CHUNK_COUNT);

			// World coordinates of chunk origin
			float worldX = (float)chunkX * VOXEL_CHUNK_SIZE;
			float worldZ = (float)chunkZ * VOXEL_CHUNK_SIZE;

			for (uint32_t z = 0; z < VOXEL_CHUNK_SIZE; ++z) {
				for (uint32_t x = 0; x < VOXEL_CHUNK_SIZE; ++x) {
					float wx = (worldX + (float)x) * NOISE_SCALE;
					float wz = (worldZ + (float)z) * NOISE_SCALE;

					// Get height from noise
					auto heightValue = perlin.normalizedOctave2D_01(wx, wz, 8);

					// Scale [0, 1] noise to [0, HEIGHT_SCALE]
					auto terrainHeight = static_cast<int32_t>(heightValue * HEIGHT_SCALE);

					// Limit height to chunk bounds
					terrainHeight = std::min(std::max(1, terrainHeight), static_cast<int32_t>(VOXEL_CHUNK_SIZE - 1));

					// TODO(piero): Keeping some code here for compatibility with voxel mesh renderer.
					for (uint32_t y = 0; y < VOXEL_CHUNK_SIZE_Y; ++y) {
						// uint32_t voxelIndex = getVoxelIndex(x, y, z, 1, 1);

						glm::vec4 voxelColor = { 0.0f, 0.0f, 0.0f, 0.0f };
						bool isVoxelActive = false;
						uint32_t materialIndex = 0;

						if (y < terrainHeight) {
							isVoxelActive = true;

							// Determine voxel type based on depth
							if (y == terrainHeight - 1) {
								// Grass
								materialIndex = 1;
								voxelColor = { 0.2f, 0.7f, 0.3f, 1.0f };
							} else if (y > terrainHeight - 4) {
								// Dirt layer (just below surface)
								materialIndex = 2;
								voxelColor = { 0.6f, 0.4f, 0.2f, 1.0f };
							} else {
								// Stone layer (deep)
								materialIndex = 3;
								voxelColor = { 0.5f, 0.5f, 0.5f, 1.0f };
							}

							if (DEBUG_CHUNK_COLORS) {
								voxelColor = { (float)(chunkX + 1) / (float)TERRAIN_DIMENSION, 0.0f, (float)(chunkZ + 1) / (float)TERRAIN_DIMENSION, 1.0f };
							}

							// Count non-empty voxels
							terrain.voxelCount++;
						}

						uint32_t vi = getVoxelIndex(x, y, z, chunkX, chunkZ);
						terrain.voxelData[vi] = materialIndex;

						// Set voxel properties
						/*
						voxelChunk.voxels[voxelIndex] = {
							.id = voxelIndex,
							.x = (uint8_t)x,
							.y = (uint8_t)y,
							.z = (uint8_t)z,
							.color = voxelColor,
							.empty = !isVoxelActive
						};
						*/
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
