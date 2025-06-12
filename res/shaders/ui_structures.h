struct UIVertex {
	vec2 position;
	vec2 uv;
	vec4 color;
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
	vec2 rectHalfSize;
	float borderThickness;
	float softness;
	float opacity;
	float padding;
	float cornerRadii[4];
};
