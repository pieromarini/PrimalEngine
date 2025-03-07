#version 450

#extension GL_GOOGLE_include_directive : require

#include "input_structures.h"

#define DEBUG 0

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in flat uint drawId;

layout (location = 0) out vec4 outFragColor;

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
	float lightValue = max(dot(inNormal, sceneData.sunlightDirection.xyz), 0.9f);

	vec3 color = inColor * texture(colorTex, inUV).xyz * sceneData.sunlightColor.xyz * lightValue;
	vec3 ambient = color * sceneData.ambientColor.xyz;

	outFragColor = vec4(color * sceneData.sunlightColor.w + ambient, 1.0f);

#if DEBUG
	uint mhash = hash(drawId);
	outFragColor = vec4(float(mhash & 255), float((mhash >> 8) & 255), float((mhash >> 16) & 255), 255) / 255.0;
#endif
}
