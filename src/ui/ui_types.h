#pragma once

#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace pm {

struct UIVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 color;
	float uv_y;
};

struct UIElementOptions {
	float width;
	float height;
	float rotation;
	glm::vec2 position;
	glm::vec2 scale;
};

struct UIMaterialData {
	glm::vec4 edgeColor;
	glm::vec4 fillColor;
};

struct UITextMaterialData {
	glm::vec4 color;
};

struct UIMaterial {
	std::string name{};
	UIMaterialData materialData{};
};

struct UITextMaterial {
	std::string name{};
	UITextMaterialData materialData{};
};

struct UIElement {
	UIElement(UIElementOptions& options): 
		position{options.position}, scale{options.scale}, rotation{options.rotation},
		width{options.width}, height{options.height}
		{}

	glm::vec2 position;
	glm::vec2 scale;
	float rotation;
	float width;
	float height;

	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t vertexOffset;

	UIMaterial materialIndex;
};

struct UITextElement {
	UITextElement(UIElementOptions& options): 
		position{options.position}, scale{options.scale}, rotation{options.rotation},
		width{options.width}, height{options.height}
		{}

	glm::vec2 position;
	glm::vec2 scale;
	float rotation;
	float width;
	float height;

	uint32_t firstIndex;
	uint32_t indexCount;
	int32_t vertexOffset;

	UITextMaterial materialIndex;
};


}// namespace pm
