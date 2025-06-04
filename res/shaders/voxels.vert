#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_ARB_shader_draw_parameters: require

#include "input_structures.h"

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outFragPos;
layout (location = 3) out flat uint outDrawId;

// Unpack position data. Each component is 5 bits.
vec3 unpackPosition(uint packed) {
	float x = float(packed & 0x1Fu);
	float y = float((packed >> 5u) & 0x1Fu);
	float z = float((packed >> 10u) & 0x1Fu);

	return vec3(x, y, z);
}

struct VoxelVertex {
	vec3 normal;
	float paddding1;
	vec3 position;
	float paddding2;
	vec4 color;
};

layout(buffer_reference, std430, buffer_reference_align = 4) readonly buffer VertexBuffer {
	VoxelVertex vertices[];
};

layout(push_constant) uniform constants {
	vec4 viewPosition;
	vec4 padding;
	vec4 padding1;
	vec4 padding2;
	VertexBuffer vertexBuffer;
} PushConstants;

layout (set = 1, binding = 0) readonly buffer DrawCommands {
	IndirectCommandData drawCommands[];
};

layout (std430, set = 1, binding = 1) readonly buffer Draws {
	MeshDraw draws[];
};

void main() {
	uint drawId = drawCommands[gl_DrawIDARB].drawId;
	MeshDraw meshDraw = draws[drawId];

	mat4 transform = meshDraw.transform;

	VoxelVertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	vec3 unpackedPos = v.position;
	vec4 position = vec4(unpackedPos, 1.0f);

	// TODO(piero): Calculate Normal matrix on CPU
	vec3 worldSpaceNormal = mat3(transpose(inverse(transform))) * v.normal;
	vec4 worldPosition = transform * position;

	gl_Position =  sceneData.viewproj * worldPosition;

	outColor = v.color.xyz;
	outNormal = worldSpaceNormal;
	outFragPos = worldPosition.xyz;
	outDrawId = drawId;
}
