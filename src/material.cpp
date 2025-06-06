#include "material.h"
#include <format>
#include <iostream>

namespace pm {

Material Material_getDefaultMaterial() {
	return {
		.name = "Default Material",
		.materialData = {
			.albedoTexture = 0,
			.normalTexture = 0,
			.specularTexture = 0,
			.emissiveTexture = 0,
			.colorFactors = { 0.85f, 0.0f, 1.0f, 1.0f },
			.metalRoughFactors = { 0.0f, 0.0f, 0.0f, 0.0f }
		}
	};
}

MaterialCache MaterialCache_init() {
	return {};
}

bool MaterialCache_add(MaterialCache& cache, MaterialIndex index, Material material) {
	if (cache.contains(index)) {
		std::cout << std::format("[MaterialCache] Cannot add material with index {} because it already exists.", index);
		return false;
	}
	cache.emplace(index, material);
	return true;
}

bool MaterialCache_remove(MaterialCache& cache, MaterialIndex index) {
	return cache.erase(index);
}

uint32_t MaterialCache_size(MaterialCache& cache) {
	return cache.size();
}

Material& MaterialCache_get(MaterialCache& cache, MaterialIndex index) {
	return cache.at(index);
}

}// namespace pm
