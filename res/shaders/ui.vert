#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;

layout (push_constant) uniform constants {
  mat4 transform;
} PushConstants;

layout (binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
} ubo;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outColor;

void main() {
  outUV = inUV;
  outColor = inColor;
  gl_Position = ubo.projection * ubo.view * PushConstants.transform * vec4(inPos, 0.0, 1.0);
}
