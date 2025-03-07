#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_ARB_shader_draw_parameters: require

#include "input_structures.h"

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out flat uint outDrawId;

struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
	Vertex vertices[];
};

layout(push_constant) uniform constants {
	uint drawIdOffset;
	VertexBuffer vertexBuffer;
} PushConstants;

layout (set = 2, binding = 0, std140) readonly buffer DrawCommands {
  IndirectCommandData drawCommands[];
};

layout (set = 2, binding = 1, std140) readonly buffer Transform {
  mat4 transforms[];
};

void main() {
  uint drawId = drawCommands[gl_DrawIDARB].drawId;
	mat4 transform = transforms[gl_DrawIDARB];
	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];
	
	vec4 position = vec4(v.position, 1.0f);

	gl_Position =  sceneData.viewproj * transform * position;

	outNormal = (transform * vec4(v.normal, 0.f)).xyz;
	outColor = v.color.xyz * materialData.colorFactors.xyz;	
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
	outDrawId = drawId;
}
