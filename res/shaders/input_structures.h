struct Vertex {
	vec3 position;
	float uv_x;
	vec3 normal;
	float uv_y;
	vec4 color;
	vec4 tangent;
};

struct IndirectCommandData {
	uint drawId;

	// VkDrawIndexedIndirectCommand
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	int vertexOffset;
	uint firstInstance;
};

struct MaterialData {
	uint albedoTexture;
	uint normalTexture;
	uint specularTexture;
	uint emissiveTexture;
	vec4 colorFactors;
	vec4 metal_rough_factors;
};

struct MeshDraw {
	mat4 transform;
	uint materialIndex;
	float padding[3];
};

layout(set = 0, binding = 0) uniform SceneData {
	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;
	vec4 sunlightDirection; //w for sun power
	vec4 sunlightColor;
} sceneData;
