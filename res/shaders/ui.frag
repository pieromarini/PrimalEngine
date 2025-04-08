#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference: require
#extension GL_ARB_shader_draw_parameters: require

#include "ui_structures.h"

layout (binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
} ubo;

layout (binding = 2) readonly buffer Draws {
  UIDraw draws[];
};

layout (binding = 3) readonly buffer Materials {
  UIMaterialData materialData[];
};

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inColor;
layout (location = 2) in flat uint drawId;

layout (location = 0) out vec4 outFragColor;

void main() {
	UIDraw draw = draws[drawId];
	UIMaterialData material = materialData[draw.materialIndex];

	if (material.backgroundColor.a <= 0.0f) {
		discard;
	}

	outFragColor = material.backgroundColor;

	// outFragColor = vec4(inColor, 1.0f);
}
