#version 450

#extension GL_GOOGLE_include_directive : require

#include "input_structures.glsl"

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() {
	float lightValue = max(dot(inNormal, sceneData.sunlightDirection.xyz), 0.9f);

	vec3 color = inColor * texture(colorTex, inUV).xyz * sceneData.sunlightColor.xyz * lightValue;
	vec3 ambient = color * sceneData.ambientColor.xyz;

	outFragColor = vec4(color * sceneData.sunlightColor.w + ambient, 1.0f);
}
