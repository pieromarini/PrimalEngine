struct UIVertex {
	vec3 position;
  float uv_x;
	vec3 color;
  float uv_y;
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

struct UIDraw {
	mat4 transform;
	uint materialIndex;
	float padding[3];
};

struct UIMaterialData {
	vec4 backgroundColor;
};
