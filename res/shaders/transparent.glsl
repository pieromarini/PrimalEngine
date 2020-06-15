// Textured shader transparency

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
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main() {             
	FragColor = texture(texture1, TexCoords);
} 
