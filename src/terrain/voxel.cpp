#include "voxel.h"
#include "glm/ext/matrix_transform.hpp"
#include "math/noise.h"
#include "vk_types.h"
#include <iostream>
#include <cmath>
#include <limits>

namespace pm {

const std::array<glm::ivec3, 8> CUBE_CORNERS = {{
    {0, 0, 0},
    {1, 0, 0},
    {0, 1, 0},
    {1, 1, 0},
    {0, 0, 1},
    {1, 0, 1},
    {0, 1, 1},
    {1, 1, 1},
}};
const std::array<glm::vec3, 8> CUBE_CORNER_VECTORS = {{
    {0.0, 0.0, 0.0},
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
    {1.0, 0.0, 1.0},
    {0.0, 1.0, 1.0},
    {1.0, 1.0, 1.0},
}};

const std::array<glm::ivec2, 12> CUBE_EDGES = {{
    {0b000, 0b001},
    {0b000, 0b010},
    {0b000, 0b100},
    {0b001, 0b011},
    {0b001, 0b101},
    {0b010, 0b011},
    {0b010, 0b110},
    {0b011, 0b111},
    {0b100, 0b101},
    {0b100, 0b110},
    {0b101, 0b111},
    {0b110, 0b111},
}};

uint32_t packPosition(uint8_t x, uint8_t y, uint8_t z) {
	return (z << 10) | (y << 5) | x;
}

void unpackPosition(uint32_t packed, uint8_t& x, uint8_t& y, uint8_t& z) {
	x = packed & 0x1F;
	y = (packed >> 5) & 0x1F;
	z = (packed >> 10) & 0x1F;
}

static const siv::PerlinNoise::seed_type seed = 728492752u;
static const siv::PerlinNoise perlin{ seed };

auto const implicit_function = [](float x, float y, float z) -> float {
	auto heightValue = 6.0f * static_cast<float>(perlin.normalizedOctave2D_01(x * 0.3f, z * 0.3f, 8));
	return static_cast<float>(y - heightValue);
};

// Calculate normal vector as the gradient of the SDF
static glm::vec3 sdfGradient(std::array<float, 8>& dists, glm::vec3& s) {
	auto p00 = glm::vec3{ dists[0b001], dists[0b010], dists[0b100] };
	auto n00 = glm::vec3{ dists[0b000], dists[0b000], dists[0b000] };

	auto p10 = glm::vec3{ dists[0b101], dists[0b011], dists[0b110] };
	auto n10 = glm::vec3{ dists[0b100], dists[0b001], dists[0b010] };

	auto p01 = glm::vec3{ dists[0b011], dists[0b110], dists[0b101] };
	auto n01 = glm::vec3{ dists[0b010], dists[0b100], dists[0b001] };

	auto p11 = glm::vec3{ dists[0b111], dists[0b111], dists[0b111] };
	auto n11 = glm::vec3{ dists[0b110], dists[0b101], dists[0b011] };

	auto d00 = p00 - n00; // Edges (0b00x, 0b0y0, 0bz00)
	auto d10 = p10 - n10; // Edges (0b10x, 0b0y1, 0bz10)
	auto d01 = p01 - n01; // Edges (0b01x, 0b1y0, 0bz01)
	auto d11 = p11 - n11; // Edges (0b11x, 0b1y1, 0bz11)

	auto neg = glm::vec3{ 1.0f } - s;

	glm::vec3 negYZX{ neg.y, neg.z, neg.x };
	glm::vec3 negZXY{ neg.z, neg.x, neg.y };

	glm::vec3 sYZX{ s.y, s.z, s.x };
	glm::vec3 sZXY{ s.z, s.x, s.y };

	// billinear interpolation between 4 edges in each dimension
	return glm::normalize(negYZX * negZXY * d00
        + negYZX * sZXY * d10
        + sYZX * negZXY * d01
        + sYZX * sZXY * d11);
}

static void tryMakeQuad(
		std::vector<float>& sdf, std::vector<uint32_t>& strideToIndex,
		std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices, 
		uint32_t p1, uint32_t p2, uint32_t axisBStride, uint32_t axisCStride) {

	auto d1 = sdf.at(p1);
	auto d2 = sdf.at(p2);
	bool negativeFace{};
	if (d1 < 0.0 && !(d2 < 0.0)) {
		negativeFace = false;
	} else if (!(d1 < 0.0) && d2 < 0.0) {
		negativeFace = true;	
	} else {
		return;
	}

	// The triangle points, viewed face-front, look like this:
	// v1 v3
	// v2 v4
	auto v1 = strideToIndex[p1];
	auto v2 = strideToIndex[p1 - axisBStride];
	auto v3 = strideToIndex[p1 - axisCStride];
	auto v4 = strideToIndex[p1 - axisBStride - axisCStride];

	auto pos1 = vertices.at(v1).position;
	auto pos2 = vertices.at(v2).position;
	auto pos3 = vertices.at(v3).position;
	auto pos4 = vertices.at(v4).position;
	// Split the quad along the shorter axis, rather than the longer one.
	if (glm::distance2(pos1, pos4) < glm::distance2(pos2, pos3)) {
		if (negativeFace) {
			indices.push_back(v1);
			indices.push_back(v4);
			indices.push_back(v2);
			indices.push_back(v1);
			indices.push_back(v3);
			indices.push_back(v4);
		} else {
			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(v4);
			indices.push_back(v1);
			indices.push_back(v4);
			indices.push_back(v3);
		}
	} else if (negativeFace) {
		indices.push_back(v2);
		indices.push_back(v3);
		indices.push_back(v4);
		indices.push_back(v2);
		indices.push_back(v1);
		indices.push_back(v3);
	} else {
		indices.push_back(v2);
		indices.push_back(v4);
		indices.push_back(v3);
		indices.push_back(v2);
		indices.push_back(v3);
		indices.push_back(v1);
	};
}


void generateTerrainGeometry(VoxelTerrain& terrain, std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices) {
	/*
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

		// Update the index count for this chunk based on actual generated faces
		chunk.indexCount = indices.size() - indicesBeforeChunk;
	}
	*/

	size_t indicesBeforeChunk = indices.size();

	// Surface nets

	uint32_t minZ = 0, minY = 0, minX = 0;
	uint32_t maxZ = VOXEL_CHUNK_SIZE, maxY = VOXEL_CHUNK_SIZE_Y, maxX = VOXEL_CHUNK_SIZE;

	std::vector<float> sdf;
	sdf.resize((VOXEL_CHUNK_SIZE + 1) * (VOXEL_CHUNK_SIZE + 1) * (VOXEL_CHUNK_SIZE + 1));

	// auxiliary structures
	std::vector<glm::vec3> surfacePoints;
	std::vector<uint32_t> surfaceStrides;
	std::vector<uint32_t> strideToIndex;
	strideToIndex.resize(sdf.size());

	for (uint32_t z = minZ; z < maxZ; ++z) {
		for (uint32_t y = minY; y < maxY; ++y) {
			for (uint32_t x = minX; x < maxX; ++x) {
				auto linearIndex = getChunkVoxelIndex(x, y, z);
				sdf.at(linearIndex) = implicit_function((float)x, (float)y, (float)z);
			}
		}
	}

	for (uint32_t z = minZ; z < maxZ; ++z) {
		for (uint32_t y = minY; y < maxY; ++y) {
			for (uint32_t x = minX; x < maxX; ++x) {
				auto linearIndex = getChunkVoxelIndex(x, y, z);
				glm::vec3 p{ x, y, z };

				std::array<float, 8> cornerDists{};
				uint32_t numNegative = 0;

				for(uint32_t i = 0; i < cornerDists.size(); ++i) {
					auto cubeCorner = CUBE_CORNERS.at(i);
					auto cubeIndex = getChunkVoxelIndex(cubeCorner.x, cubeCorner.y, cubeCorner.z);
					auto d = sdf.at(linearIndex + cubeIndex);
					cornerDists.at(i) = d;
					if (d < 0.0f) {
						numNegative++;
					}
				}

				// no edge crossings
				if (numNegative == 0 || numNegative == 8) {
					strideToIndex.at(linearIndex) = std::numeric_limits<uint32_t>::max();
					continue;
				}

				uint32_t count = 0;
				glm::vec3 sum{ 0.0f };

				for (auto& edge : CUBE_EDGES) {
					auto d1 = cornerDists.at(edge.x);
					auto d2 = cornerDists.at(edge.y);

					// Detect 0 crossing
					if ((d1 < 0.0) != (d2 < 0.0)) {
						count++;

						// edge interpolation
						float interp1 = d1 / (d1 - d2);
						float interp2 = 1.0f - interp1;
						sum += interp2 * CUBE_CORNER_VECTORS.at(edge.x) + interp1 * CUBE_CORNER_VECTORS.at(edge.y);
					}
				}

				// get centroid
				auto c = sum * (1.0f / (float)count);

				vertices.push_back({ .normal = sdfGradient(cornerDists, c), .position = p + c, .color = { 0.42f, 0.42f, 0.42f, 1.0f } });
				strideToIndex.at(linearIndex) = vertices.size() - 1;
				surfacePoints.emplace_back(x, y, z);
				surfaceStrides.push_back(linearIndex);
			}
		}
	}

	// Generate triangles
	glm::vec3 xyzStrides = {
		getChunkVoxelIndex(1, 0, 0),
		getChunkVoxelIndex(0, 1, 0),
		getChunkVoxelIndex(0, 0, 1)
	};

	for (uint32_t i = 0; i < surfacePoints.size(); ++i) {
		auto sp = surfacePoints.at(i);
		auto stride = surfaceStrides.at(i);

		auto x = sp.x;
		auto y = sp.y;
		auto z = sp.z;

		if ((y != minY) && (z != minZ) && (x != (maxX - 1))) {
			tryMakeQuad(sdf, strideToIndex, vertices, indices, stride, stride + xyzStrides.x, xyzStrides.y, xyzStrides.z);
		}

		if ((x != minX) && (z != minZ) && (y != (maxY - 1))) {
			tryMakeQuad(sdf, strideToIndex, vertices, indices, stride, stride + xyzStrides.y, xyzStrides.z, xyzStrides.x);
		}

		if ((x != minX) && (y != minY) && (z != (maxZ - 1))) {
			tryMakeQuad(sdf, strideToIndex, vertices, indices, stride, stride + xyzStrides.z, xyzStrides.x, xyzStrides.y);
		}
	}
	assert(terrain.chunks.size() == 1);
	terrain.chunks[0].indexCount = indices.size() - indicesBeforeChunk;
}

// returns an index relative to the world
uint32_t getVoxelIndex(uint32_t localX, uint32_t y, uint32_t localZ, uint32_t chunkX, uint32_t chunkZ) {
	uint32_t globalX = localX + (chunkX * VOXEL_CHUNK_SIZE);
	uint32_t globalZ = localZ + (chunkZ * VOXEL_CHUNK_SIZE);

	uint32_t totalXSize = TERRAIN_DIMENSION * VOXEL_CHUNK_SIZE;
	uint32_t totalZSize = TERRAIN_DIMENSION * VOXEL_CHUNK_SIZE;

	return globalX + (globalZ * totalXSize) + (y * totalXSize * totalZSize);
}

// returns an index relative to a chunk
uint32_t getChunkVoxelIndex(uint32_t x, uint32_t y, uint32_t z) {
	return x + (z * VOXEL_CHUNK_SIZE) + (y * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE);
}

VoxelTerrain generateTerrain() {
	VoxelTerrain terrain{};

	// Create noise generators with different seeds
	const siv::PerlinNoise::seed_type seed = 728492752u;
	const siv::PerlinNoise perlin{ seed };

	// Terrain generation parameters
	constexpr float HEIGHT_SCALE = 35.0f;
	constexpr float NOISE_SCALE = 0.03f;

	terrain.grid.minBound = glm::vec3(0.0f, 0.0f, 0.0f);
	terrain.grid.maxBound = glm::vec3(VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION, VOXEL_CHUNK_SIZE_Y, VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION);
	terrain.grid.gridSize = terrain.grid.maxBound - terrain.grid.minBound;
	terrain.grid.numVoxelsX = VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION;
	terrain.grid.numVoxelsY = VOXEL_CHUNK_SIZE_Y;
	terrain.grid.numVoxelsZ = VOXEL_CHUNK_SIZE * TERRAIN_DIMENSION;

	// Use for raytraced rendering of voxels
	// terrain.voxelData.resize(VOXEL_CHUNK_COUNT * TERRAIN_DIMENSION * TERRAIN_DIMENSION);

	for (uint32_t chunkZ = 0; chunkZ < TERRAIN_DIMENSION; ++chunkZ) {
		for (uint32_t chunkX = 0; chunkX < TERRAIN_DIMENSION; ++chunkX) {
			VoxelChunk voxelChunk{};
			voxelChunk.voxels.resize(VOXEL_CHUNK_COUNT);

			// World coordinates of chunk origin
			float worldX = (float)chunkX * VOXEL_CHUNK_SIZE;
			float worldZ = (float)chunkZ * VOXEL_CHUNK_SIZE;

			/*
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

						// uint32_t vi = getVoxelIndex(x, y, z, chunkX, chunkZ);
						// terrain.voxelData[vi] = materialIndex;

						uint32_t vi = getChunkVoxelIndex(x, y, z);
						voxelChunk.voxels[vi] = {
							.id = vi,
							.x = (uint8_t)x,
							.y = (uint8_t)y,
							.z = (uint8_t)z,
							// .value = implicit_function(x, y, z),
							.color = voxelColor,
							.empty = !isVoxelActive
						};
					}
				}
			}
			*/

			voxelChunk.transform = glm::translate(glm::mat4{ 1.0f }, glm::vec3{ chunkX * VOXEL_CHUNK_SIZE, 0.0f, chunkZ * VOXEL_CHUNK_SIZE });
			terrain.chunks.push_back(voxelChunk);
		}
	}

	return terrain;
}
}// namespace pm
