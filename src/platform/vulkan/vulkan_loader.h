#pragma once

#include "vk_types.h"
#include <filesystem>

namespace pm {

struct GLTFMaterial {
	MaterialInstance data;
};

struct GeoSurface {
	uint32_t startIndex;
	uint32_t count;
	std::shared_ptr<GLTFMaterial> material;
};

struct MeshAsset {
		std::string name;

		std::vector<GeoSurface> surfaces;
		GPUMeshBuffers meshBuffers;
};


class VulkanRenderer;

std::optional<std::vector<std::shared_ptr<MeshAsset>>> loadGltfMeshes(pm::VulkanRenderer* engine, std::filesystem::path filePath);

}
