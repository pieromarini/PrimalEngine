#include "voxel.h"
#include "glm/ext/matrix_transform.hpp"

namespace pm {

uint32_t packPosition(uint8_t x, uint8_t y, uint8_t z) {
	// Pack the values: x in bits 0-4, y in bits 5-9, z in bits 10-14
	return (z << 10) | (y << 5) | x;
}

void unpackPosition(uint32_t packed, uint8_t& x, uint8_t& y, uint8_t& z) {
	x = packed & 0x1F;
	y = (packed >> 5) & 0x1F;
	z = (packed >> 10) & 0x1F;
}

void generateTerrainGeometry(VoxelTerrain& terrain, std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices) {
	for (auto& chunk : terrain.chunks) {
		chunk.firstIndex = indices.size();
		chunk.vertexOffset = static_cast<int32_t>(vertices.size());
		chunk.indexCount = 6 * 6 * VOXEL_CHUNK_COUNT;

		for (auto& voxel : chunk.voxels) {
			uint8_t size = VOXEL_SIZE;

			// Vertex indices for the current voxel
			auto baseIndex = static_cast<uint32_t>(vertices.size());
			auto color = voxel.color;

			// Front face (Z+)
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x - size, voxel.y - size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x + size, voxel.y - size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, 1.0f), .data = packPosition(voxel.x - size, voxel.y + size, voxel.z + size), .color = color });

			// Back face (Z-)
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x - size, voxel.y - size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x - size, voxel.y + size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 0.0f, -1.0f), .data = packPosition(voxel.x + size, voxel.y - size, voxel.z - size), .color = color });

			// Right face (X+)
			vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y - size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y - size, voxel.z + size), .color = color });

			// Left face (X-)
			vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y - size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y - size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y + size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y + size, voxel.z - size), .color = color });

			// Top face (Y+)
			vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y + size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y + size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, 1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y + size, voxel.z - size), .color = color });

			// Bottom face (Y-)
			vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y - size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y - size, voxel.z - size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x + size, voxel.y - size, voxel.z + size), .color = color });
			vertices.push_back({ .normal = glm::vec3(0.0f, -1.0f, 0.0f), .data = packPosition(voxel.x - size, voxel.y - size, voxel.z + size), .color = color });

			// Add indices for the 6 faces
			for (int face = 0; face < 6; face++) {
				uint32_t faceBaseIndex = baseIndex + face * 4;
				indices.push_back(faceBaseIndex);
				indices.push_back(faceBaseIndex + 1);
				indices.push_back(faceBaseIndex + 2);

				indices.push_back(faceBaseIndex);
				indices.push_back(faceBaseIndex + 2);
				indices.push_back(faceBaseIndex + 3);
			}
		}
	}
}

VoxelTerrain generateTerrain() {
	VoxelTerrain terrain{};

	constexpr uint32_t TERRAIN_DIMENSION = 4;

	for (uint32_t chunkZ = 0; chunkZ < TERRAIN_DIMENSION; ++chunkZ) {
		for (uint32_t chunkX = 0; chunkX < TERRAIN_DIMENSION; ++chunkX) {
			VoxelChunk voxelChunk{};
			voxelChunk.voxels.resize(VOXEL_CHUNK_COUNT);

			for (uint8_t y = 0; y < VOXEL_CHUNK_SIZE; ++y) {
				for (uint8_t z = 0; z < VOXEL_CHUNK_SIZE; ++z) {
					for (uint8_t x = 0; x < VOXEL_CHUNK_SIZE; ++x) {
						uint32_t voxelIndex = x + (z * VOXEL_CHUNK_SIZE) + (y * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE);
						voxelChunk.voxels[voxelIndex] = {
							.id = voxelIndex,
							.x = x,
							.y = y,
							.z = z,
							.color = { 0.0f, 0.0f, 1.0f, 1.0f }
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
