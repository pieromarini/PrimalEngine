#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference: require
#extension GL_ARB_shader_draw_parameters: require

#include "ui_structures.h"

layout (buffer_reference , std430, buffer_reference_align=8) readonly buffer UIVertexBuffer {
  UIVertex vertices[];
};

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

	// NOTE(piero): Rendering a border for rectangles
	if ((material.horizontalBorder > 0.0f || material.verticalBorder > 0.0f) && 
			(inUV.x < material.horizontalBorder || inUV.x > (1.0f - material.horizontalBorder) || 
			inUV.y < material.verticalBorder || inUV.y > (1.0f - material.verticalBorder))) {
		outFragColor = vec4(0.0f, 0.0f, 1.0, 1.0f);
	} else {
		if (material.backgroundColor.a <= 0.0f) {
			discard;
		}
		outFragColor = material.backgroundColor;
	}

	// outFragColor = vec4(inColor, 1.0f);
}
