#version 450

#extension GL_EXT_buffer_reference: require
#extension GL_ARB_shader_draw_parameters: require

struct UIVertex {
	vec3 position;
  float uv_x;
	vec3 color;
  float uv_y;
};

struct IndirectCommandData {
  uint drawId;

	// VkDrawIndexedIndirectCommand
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	int vertexOffset;
	uint firstInstance;
};

layout (buffer_reference , std430, buffer_reference_align=8) readonly buffer UIVertexBuffer {
  UIVertex vertices[];
};

layout (push_constant) uniform constants {
  mat4 transform;
  UIVertexBuffer vertexBuffer;
} PushConstants;

layout (binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
} ubo;

layout (std140, binding = 1) readonly buffer DrawCommands {
  IndirectCommandData drawCommands[];
};

layout (std140, binding = 2) readonly buffer Transform {
  mat4 transforms[];
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outColor;

void main() {
  // TODO: This always returns 0
  //       The whole `IndirectCommandData` structure is returned with 0's when inspected in RenderDoc
  //       but when inspecting the actual buffer, the data is present.
  //       For now, using gl_DrawIDARB instead just works, but maybe we want to rely on drawId later on.
  uint cmds = drawCommands[gl_DrawIDARB].drawId;
  UIVertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

  outUV = vec2(v.uv_x, v.uv_y);
  outColor = v.color;
  gl_Position = ubo.projection * ubo.view * transforms[gl_DrawIDARB] * vec4(v.position.xy, 0.0, 1.0);
}
