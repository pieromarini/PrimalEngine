#pragma once

#include "vk_types.h"
#include <filesystem>

struct GeoSurface {
		uint32_t startIndex;
		uint32_t count;
};

struct MeshAsset {
		std::string name;

		std::vector<GeoSurface> surfaces;
		GPUMeshBuffers meshBuffers;
};

namespace pm {
class VulkanRenderer;
}

std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(pm::VulkanRenderer* engine, std::filesystem::path filePath);
