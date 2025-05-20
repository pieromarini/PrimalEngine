#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_nonuniform_qualifier : require

#include "input_structures.h"

#define DEBUG 0

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in flat uint drawId;
layout (location = 4) in vec3 inFragPos;

layout (location = 0) out vec4 outFragColor;

layout(buffer_reference, std430) readonly buffer VertexBuffer {
	Vertex vertices[];
};

layout(push_constant) uniform constants {
	vec4 viewPosition;
	vec4 padding;
	vec4 padding1;
	vec4 padding2;
	VertexBuffer vertexBuffer;
} PushConstants;

layout (set = 1, binding = 0) uniform sampler2D textures[];

layout (set = 2, binding = 0) readonly buffer GLTFMaterialData {
	MaterialData materialData[];
};

layout (set = 2, binding = 1) readonly buffer DrawCommands {
	IndirectCommandData drawCommands[];
};

layout (std430, set = 2, binding = 2) readonly buffer Draws {
	MeshDraw draws[];
};

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
	MeshDraw meshDraw = draws[drawId];
	MaterialData material = materialData[meshDraw.materialIndex];

	/*
		 float lightValue = max(dot(inNormal, sceneData.sunlightDirection.xyz), 0.9f);

		 vec3 color = inColor * sceneData.sunlightColor.xyz * lightValue;
		 if (material.albedoTexture > 0) {
		 color *= texture(textures[nonuniformEXT(material.albedoTexture)], inUV).xyz;
		 }

		 vec3 ambient = color * sceneData.ambientColor.xyz;
	 */
	vec3 sunlightDirection = normalize(sceneData.sunlightDirection.xyz);

	vec3 color = vec3(1.0f);
	if (material.albedoTexture > 0) {
		color *= texture(textures[nonuniformEXT(material.albedoTexture)], inUV).xyz;
	}

	vec3 ambient = color * 0.15 * sceneData.ambientColor.xyz;

	vec3 normal = normalize(inNormal);
	float diff = max(dot(sunlightDirection, normal), 0.0f);
	vec3 diffuse = diff * sceneData.sunlightColor.xyz * color;

	vec3 viewDir = normalize(PushConstants.viewPosition.xyz - inFragPos);

	vec3 halfwayDir = normalize(sunlightDirection + viewDir);  
	float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	vec3 specular = vec3(0.3) * spec;

	outFragColor = vec4(ambient + diffuse + specular, 1.0f);

#if DEBUG
	uint mhash = hash(drawId);
	outFragColor = vec4(float(mhash & 255), float((mhash >> 8) & 255), float((mhash >> 16) & 255), 255) / 255.0;
#endif
}
