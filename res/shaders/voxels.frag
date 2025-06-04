#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

#include "input_structures.h"

#define DEBUG 0
#define DEBUG_NORMALS 0

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inFragPos;
layout (location = 3) in flat uint drawId;

layout (location = 0) out vec4 outFragColor;

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

uint hash(uint a) {
	a = (a+0x7ed55d16) + (a<<12);
	a = (a^0xc761c23c) ^ (a>>19);
	a = (a+0x165667b1) + (a<<5);
	a = (a+0xd3a2646c) ^ (a<<9);
	a = (a+0xfd7046c5) + (a<<3);
	a = (a^0xb55a4f09) ^ (a>>16);
	return a;
}

void main() {
	// constants
	float ambientStrength = 0.5f;

	vec3 normal = normalize(inNormal);

	// ambient
	vec3 ambient = ambientStrength * sceneData.ambientColor.xyz;

	// diffuse
	float diff = max(dot(normal, normalize(sceneData.sunlightDirection.xyz)), 0.0f);
	vec3 diffuse = diff * sceneData.sunlightColor.xyz;

	vec3 result = (ambient + diffuse) * inColor;

	outFragColor = vec4(result, 1.0f);

#if DEBUG_NORMALS
	outFragColor = vec4(normal * 0.5f + 0.5f, 1.0f);
#endif

#if DEBUG
	uint mhash = hash(drawId);
	outFragColor = vec4(float(mhash & 255), float((mhash >> 8) & 255), float((mhash >> 16) & 255), 255) / 255.0;
#endif
}
