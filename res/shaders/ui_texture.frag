#version 450

#extension GL_EXT_nonuniform_qualifier : require

struct ViewportDrawData {
	mat4 transform;
	uint textureIndex;
	float padding[3];
};

layout (set = 0, binding = 2, std430) readonly buffer Draws {
	ViewportDrawData draws[];
};

layout (set = 1, binding = 0) uniform sampler2D viewportTextures[];

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;
layout (location = 2) in flat uint drawId;

layout (location = 0) out vec4 outFragColor;

void main() {
	ViewportDrawData draw = draws[drawId];
	outFragColor = texture(viewportTextures[nonuniformEXT(draw.textureIndex)], inUV);
}
