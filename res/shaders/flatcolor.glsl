// Flat Color Shader with Ambient and Diffuse Lighting

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 Normal;
out vec3 FragPos;

void main() {
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
    FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
	Normal = a_Normal;
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 u_ObjectColor;
uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;

void main() {
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * u_LightColor;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(u_LightPosition - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * u_LightColor;

	vec3 result = (ambient + diffuse) * u_ObjectColor;
	color = vec4(result, 1.0);
}
