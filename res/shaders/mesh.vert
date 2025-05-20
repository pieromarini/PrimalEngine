#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require
#extension GL_ARB_shader_draw_parameters: require

#include "input_structures.h"

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out flat uint outDrawId;
layout (location = 4) out vec3 outFragPos;

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

layout (set = 2, binding = 0) readonly buffer GLTFMaterialData {
	MaterialData materialData[];
};

layout (set = 2, binding = 1) readonly buffer DrawCommands {
	IndirectCommandData drawCommands[];
};

layout (std430, set = 2, binding = 2) readonly buffer Draws {
	MeshDraw draws[];
};

void main() {
	uint drawId = drawCommands[gl_DrawIDARB].drawId;
	MeshDraw meshDraw = draws[drawId];

	mat4 transform = meshDraw.transform;
	uint materialIndex = meshDraw.materialIndex;

	Vertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

	vec4 position = vec4(v.position, 1.0f);

	gl_Position =  sceneData.viewproj * transform * position;

	outNormal = (transform * vec4(v.normal, 0.f)).xyz;
	outColor = v.color.xyz * materialData[materialIndex].colorFactors.xyz;
	outUV.x = v.uv_x;
	outUV.y = v.uv_y;
	outDrawId = drawId;
	outFragPos = v.position;
}
