#include "voxel.h"

namespace pm {

void generateTerrainGeometry(VoxelTerrain& terrain, std::vector<VoxelVertex>& vertices, std::vector<uint32_t>& indices) {
	for (auto& chunk : terrain.chunks) {
		chunk.firstIndex = indices.size();
		chunk.vertexOffset = static_cast<int32_t>(vertices.size());
		chunk.indexCount = 6 * 6 * VOXEL_CHUNK_COUNT;

		for (auto& voxel : chunk.voxels) {
			glm::vec3 pos{ voxel.x, voxel.y, voxel.z };
			float size = VOXEL_SIZE;

			// Vertex indices for the current voxel
			auto baseIndex = static_cast<uint32_t>(vertices.size());
			auto color = voxel.color;

			// Front face (Z+)
			vertices.push_back({ .position = pos + glm::vec3(-size, -size, size), .normal = glm::vec3(0.0f, 0.0f, 1.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, -size, size), .normal = glm::vec3(0.0f, 0.0f, 1.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, size, size), .normal = glm::vec3(0.0f, 0.0f, 1.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(-size, size, size), .normal = glm::vec3(0.0f, 0.0f, 1.0f), .color = color });

			// Back face (Z-)
			vertices.push_back({ .position = pos + glm::vec3(-size, -size, -size), .normal = glm::vec3(0.0f, 0.0f, -1.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(-size, size, -size), .normal = glm::vec3(0.0f, 0.0f, -1.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, size, -size), .normal = glm::vec3(0.0f, 0.0f, -1.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, -size, -size), .normal = glm::vec3(0.0f, 0.0f, -1.0f), .color = color });

			// Right face (X+)
			vertices.push_back({ .position = pos + glm::vec3(size, -size, -size), .normal = glm::vec3(1.0f, 0.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, size, -size), .normal = glm::vec3(1.0f, 0.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, size, size), .normal = glm::vec3(1.0f, 0.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, -size, size), .normal = glm::vec3(1.0f, 0.0f, 0.0f), .color = color });

			// Left face (X-)
			vertices.push_back({ .position = pos + glm::vec3(-size, -size, -size), .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(-size, -size, size), .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(-size, size, size), .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(-size, size, -size), .normal = glm::vec3(-1.0f, 0.0f, 0.0f), .color = color });

			// Top face (Y+)
			vertices.push_back({ .position = pos + glm::vec3(-size, size, -size), .normal = glm::vec3(0.0f, 1.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(-size, size, size), .normal = glm::vec3(0.0f, 1.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, size, size), .normal = glm::vec3(0.0f, 1.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, size, -size), .normal = glm::vec3(0.0f, 1.0f, 0.0f), .color = color });

			// Bottom face (Y-)
			vertices.push_back({ .position = pos + glm::vec3(-size, -size, -size), .normal = glm::vec3(0.0f, -1.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, -size, -size), .normal = glm::vec3(0.0f, -1.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(size, -size, size), .normal = glm::vec3(0.0f, -1.0f, 0.0f), .color = color });
			vertices.push_back({ .position = pos + glm::vec3(-size, -size, size), .normal = glm::vec3(0.0f, -1.0f, 0.0f), .color = color });

			// Add indices for the 6 faces (12 triangles)
			for (int face = 0; face < 6; face++) {
				uint32_t faceBaseIndex = baseIndex + face * 4;
				// First triangle of the face
				indices.push_back(faceBaseIndex);
				indices.push_back(faceBaseIndex + 1);
				indices.push_back(faceBaseIndex + 2);
				// Second triangle of the face
				indices.push_back(faceBaseIndex);
				indices.push_back(faceBaseIndex + 2);
				indices.push_back(faceBaseIndex + 3);
			}
		}
	}
}

VoxelTerrain generateTerrain() {
	VoxelTerrain scene{};

	for (uint32_t chunk = 0; chunk < 10; ++chunk) {
		VoxelChunk voxelChunk{};

		for (uint32_t x = 0; x < VOXEL_CHUNK_SIZE; ++x) {
			for (uint32_t y = 0; y < VOXEL_CHUNK_SIZE; ++y) {
				for (uint32_t z = 0; z < VOXEL_CHUNK_SIZE; ++z) {
					uint32_t voxelIndex = x + (y * VOXEL_CHUNK_SIZE) + (z * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE);
					voxelChunk.voxels[voxelIndex] = { .id = voxelIndex, .x = (float)(chunk * VOXEL_CHUNK_SIZE) + (float)x, .y = (float)(chunk * VOXEL_CHUNK_SIZE) + (float)y, .z = (float)(chunk * VOXEL_CHUNK_SIZE) + (float)z, .color = { 0.0f, 0.0f, 1.0f, 1.0f } };
				}
			}
		}

		voxelChunk.transform = glm::mat4{ 1.0f };

		scene.chunks.push_back(voxelChunk);
	}

	return scene;
}

}// namespace pm
