#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference: require
#extension GL_ARB_shader_draw_parameters: require

#include "ui_structures.h"

struct ViewportDrawData {
	mat4 transform;
	uint textureIndex;
	float padding[3];
};

layout (buffer_reference , std430, buffer_reference_align=8) readonly buffer UIVertexBuffer {
	UIVertex vertices[];
};

layout (push_constant) uniform constants {
	UIVertexBuffer vertexBuffer;
	float screenWidth;
	float screenHeight;
} PushConstants;

layout (set = 0, binding = 0) uniform UBO {
	mat4 projection;
	mat4 view;
} ubo;


layout (set = 0, binding = 1) readonly buffer DrawCommands {
	IndirectCommandData drawCommands[];
};

layout (set = 0, binding = 2, std430) readonly buffer Draws {
	ViewportDrawData draws[];
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;
layout (location = 2) out flat uint outDrawId;

void main() {
	uint drawId = drawCommands[gl_DrawIDARB].drawId;
	UIVertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	ViewportDrawData draw = draws[drawId];

	outDrawId = drawId;
	outUV = v.uv;
	outColor = v.color;
	gl_Position = ubo.projection * ubo.view * draw.transform * vec4(v.position.xy, 0.0, 1.0);
}
