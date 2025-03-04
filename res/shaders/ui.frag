#version 450

layout (binding = 0) uniform UBO {
  mat4 projection;
  mat4 view;
} ubo;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec4 outFragColor;

void main() {
	outFragColor = vec4(inColor, 1.0f);
}
