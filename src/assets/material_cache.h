#pragma once

#include "renderer/material.h"

namespace pm {

using MaterialIndex = uint32_t;

using MaterialCache = std::unordered_map<MaterialIndex, Material>;

Material Material_getDefaultMaterial();

MaterialCache MaterialCache_init();
bool MaterialCache_add(MaterialCache& cache, MaterialIndex index, Material material);
bool MaterialCache_remove(MaterialCache& cache, MaterialIndex index);
uint32_t MaterialCache_size(MaterialCache& cache);

Material& MaterialCache_get(MaterialCache& cache, MaterialIndex index);

}
