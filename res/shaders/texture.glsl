// Main shader

#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoords;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Bitangent;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform mat4 u_LightSpaceMatrix;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out vec4 FragPosLightSpace;

void main() {
    FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
    Normal = transpose(inverse(mat3(u_Transform))) * a_Normal;
	TexCoords = a_TexCoords;
	FragPosLightSpace = u_LightSpaceMatrix * vec4(FragPos, 1.0);
	gl_Position = u_ViewProjection * vec4(FragPos, 1.0);
}

#type fragment
#version 330 core

// NOTE: For now, materials only hold 1 texture of each type.
struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
    sampler2D texture_height1;
    float shininess;
};

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_POINT_LIGHTS 4

layout(location = 0) out vec4 color;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform vec3 u_ViewPos;
uniform DirectionalLight u_DirLight;
uniform int u_numPointLights;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform Material u_Material;

uniform sampler2D u_ShadowMap;

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);  
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);  
float CalcShadows(vec4 fragPosLightSapce, vec3 lightDirection);

void main() {
	vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(u_ViewPos - FragPos);

    vec3 result = CalcDirectionalLight(u_DirLight, norm, viewDir);

    for(int i = 0; i < u_numPointLights; i++)
        result += CalcPointLight(u_PointLights[i], norm, FragPos, viewDir);    

    color = vec4(result, 1.0);
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
	vec3 color = texture(u_Material.texture_diffuse1, TexCoords).rgb;

    vec3 lightDir = normalize(light.direction - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);

	vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);

	float shadow = CalcShadows(FragPosLightSpace, lightDir);

    vec3 ambient  = light.ambient * color;
    vec3 diffuse  = light.diffuse  * diff * color;
    vec3 specular = light.specular * spec * texture(u_Material.texture_specular1, TexCoords).rgb;

    return (ambient + (1.0 - shadow) * (diffuse + specular));
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
	vec3 color = texture(u_Material.texture_diffuse1, TexCoords).rgb;

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);

	vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    vec3 ambient  = light.ambient * color;
    vec3 diffuse  = light.diffuse  * diff * color;
    vec3 specular = light.specular * spec * texture(u_Material.texture_specular1, TexCoords).rgb;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

float CalcShadows(vec4 fragPosLightSpace, vec3 lightDirection) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(u_ShadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    vec3 normal = normalize(Normal);
    float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}
