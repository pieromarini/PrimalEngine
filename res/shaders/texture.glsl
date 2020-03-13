// Flat Color Shader with Ambient and Diffuse Lighting

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

void main() {
    FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
    Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;  
	TexCoords = a_TexCoords;

	gl_Position = u_ViewProjection * vec4(FragPos, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform vec3 u_LightColor;
uniform vec3 u_LightPosition;
uniform sampler2D texture_diffuse1;

void main() {
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * u_LightColor * vec3(texture(texture_diffuse1, TexCoords));

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(u_LightPosition - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * u_LightColor * vec3(texture(texture_diffuse1, TexCoords));

	// TODO: Add specular calculation

	vec3 result = ambient + diffuse;
	color = vec4(result, 1.0);
}
