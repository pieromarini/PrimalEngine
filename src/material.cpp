#include "material.h"

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

}// namespace pm
