#version 450
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference: require
#extension GL_ARB_shader_draw_parameters: require

#include "ui_structures.h"

layout (buffer_reference , std430, buffer_reference_align=8) readonly buffer UIVertexBuffer {
	UIVertex vertices[];
};

layout (set = 0, binding = 0) uniform UBO {
	mat4 projection;
	mat4 view;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D albedoTexture;

layout (set = 0, binding = 2) readonly buffer Draws {
	UIDraw draws[];
};

layout (set = 0, binding = 3) readonly buffer Materials {
	UIMaterialData materialData[];
};

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;
layout (location = 2) in flat uint drawId;

layout (location = 0) out vec4 outFragColor;

// Rectangle SDF similar to the reference shader
/*
float rectSDF(vec2 p, vec2 halfSize, float cornerRadius) {
	vec2 q = abs(p) - halfSize + cornerRadius;
	return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - cornerRadius;
}
*/

float rectSDF(vec2 samplePos, vec2 rectHalfSize, float r) {
	return length(max(abs(samplePos) - rectHalfSize + r, 0.0)) - r;
}

void main() {
	UIDraw draw = draws[drawId];
	UIMaterialData material = materialData[draw.materialIndex];

	vec2 uv = inUV;

	vec4 color00 = inColor;
	vec4 color01 = inColor;
	vec4 color10 = inColor;
	vec4 color11 = inColor;
	
	// Lerp the 4 colors to create a gradient
	vec4 topColor = (1.0 - uv.x) * color00 + uv.x * color10;
	vec4 botColor = (1.0 - uv.x) * color01 + uv.x * color11;
	vec4 tint = (1.0 - uv.y) * topColor + uv.y * botColor;
	
	vec4 albedoSample = vec4(1.0, 1.0, 1.0, 1.0);
	
	// Determine SDF sample position - convert (0-1) to rect pixel coordinates
	vec2 sdfSamplePos = vec2(
		(2.0 * uv.x - 1.0) * material.rectHalfSize.x,
		(2.0 * uv.y - 1.0) * material.rectHalfSize.y
	);
	
	// Get corner radius for current position
	float cornerRadius;
	if (uv.x < 0.5 && uv.y < 0.5) {
		cornerRadius = material.cornerRadii[0]; // bottom-left (00)
	} else if (uv.x >= 0.5 && uv.y < 0.5) {
		cornerRadius = material.cornerRadii[1]; // bottom-right (10)
	} else if (uv.x >= 0.5 && uv.y >= 0.5) {
		cornerRadius = material.cornerRadii[2]; // top-right (11)
	} else {
		cornerRadius = material.cornerRadii[3]; // top-left (01)
	}
	
	vec2 cornerSampleSize = material.rectHalfSize - vec2(material.softness * 2.0);
	float cornerSdfS = rectSDF(sdfSamplePos, cornerSampleSize, cornerRadius);
	float cornerSdfT = 1.0 - smoothstep(0.0, 2.0 * material.softness, cornerSdfS);
	
	// Sample for borders
	float borderSdfT = 1.0;
	if (material.borderThickness > 0.0) {
		vec2 borderSampleSize = cornerSampleSize - vec2(material.borderThickness);
		float borderRadius = max(cornerRadius - material.borderThickness, 0.0);
		float borderSdfS = rectSDF(sdfSamplePos, borderSampleSize, borderRadius);
		borderSdfT = smoothstep(0.0, 2.0 * material.softness, borderSdfS);
	}
	
	// Form and return final color
	vec4 finalColor = albedoSample;
	finalColor *= tint;
	finalColor *= material.opacity;
	finalColor *= cornerSdfT;
	finalColor *= borderSdfT;
	
	outFragColor = finalColor;
}
