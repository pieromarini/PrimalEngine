#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference: require
#extension GL_ARB_shader_draw_parameters: require

#include "ui_structures.h"

struct FontDraw {
	vec4 textColor;
};

layout (buffer_reference , std430, buffer_reference_align=8) readonly buffer UIVertexBuffer {
	UIVertex vertices[];
};

layout (push_constant) uniform constants {
	UIVertexBuffer vertexBuffer;
} PushConstants;

layout (set = 0, binding = 0) uniform UBO {
	mat4 projection;
	mat4 view;
	float pxRange;
} ubo;


layout (set = 0, binding = 2, std430) readonly buffer DrawCommands {
	IndirectCommandData drawCommands[];
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;
layout (location = 2) out flat uint outDrawId;

void main() {
	uint drawId = drawCommands[gl_DrawIDARB].drawId;
	UIVertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	outUV = v.uv;
	outColor = v.color;
	outDrawId = drawId;
	gl_Position = ubo.projection * ubo.view * vec4(v.position.xy, 0.0, 1.0);
}
