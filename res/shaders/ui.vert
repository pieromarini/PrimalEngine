#version 450

#extension GL_EXT_buffer_reference: require

struct UIVertex {
	vec3 position;
  float uv_x;
	vec3 color;
  float uv_y;
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

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outColor;

void main() {
  UIVertex v = PushConstants.vertexBuffer.vertices[gl_VertexIndex];

  outUV = vec2(v.uv_x, v.uv_y);
  outColor = v.color;
  gl_Position = ubo.projection * ubo.view * PushConstants.transform * vec4(v.position.xy, 0.0, 1.0);
}
