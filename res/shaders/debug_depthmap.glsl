// Used to render a depth map to a quad

#type vertex
#version 330 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoords;

out vec2 TexCoords;

void main() {
	TexCoords = a_TexCoords;
	gl_Position = vec4(a_Position.x, a_Position.y, 0.0, 1.0); 
}


#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 TexCoords;

uniform sampler2D u_DepthMap;
uniform float u_NearPlane;
uniform float u_FarPlane;

// Used when using a perspective projection matrix when rendering the depth map
float LinearizeDepth(float depth) {
	float z = depth * 2.0 - 1.0; // Back to NDC
	return (2.0 * u_NearPlane * u_FarPlane) / (u_FarPlane + u_NearPlane - z * (u_FarPlane - u_NearPlane));
}

void main() {
	float depthValue = texture(u_DepthMap, TexCoords).r;
	// color = vec4(vec3(LinearizeDepth(depthValue) / u_FarPlane), 1.0); // perspective
	color = vec4(vec3(depthValue), 1.0); // orthographic
}
