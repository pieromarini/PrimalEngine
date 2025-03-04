#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;

layout (push_constant) uniform constants {
  mat4 transform;
} PushConstants;

layout (binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
  vec4 outlineColor;
  float outlineWidth;
  float outline;
} ubo;

layout (location = 0) out vec2 outUV;

void main() {
  outUV = inUV;
  gl_Position = ubo.projection * ubo.view * PushConstants.transform * vec4(inPos.xy, 0.0, 1.0);
}
