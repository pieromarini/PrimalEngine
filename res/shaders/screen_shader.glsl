// Basic shader used to render to a quad from a framebuffer

#type vertex
#version 330 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoords;

out vec2 TexCoords;

void main() {
	gl_Position = vec4(a_Position.x, a_Position.y, 0.0, 1.0); 
	TexCoords = a_TexCoords;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() { 
	color = texture(screenTexture, TexCoords);
}
