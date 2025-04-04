#pragma once

#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

namespace pm::UI {

enum UILayoutDirection {
	VERTICAL,
	HORIZONTAL
};

enum UISizingMode {
	STATIC = 0,
	GROW,
	FIT
};

enum UIRenderCommandType {
	RECTANGLE,
	TEXT
};

struct UIAxisSize {
	float size;
	UISizingMode sizingMode;
};

struct BoundingBox {
	float x, y;
	float width, height;
};

struct UIVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 color;
	float uv_y;
};

struct UIPadding {
	UIPadding(): top{0}, bottom{0}, left{0}, right{0} {}
	UIPadding(float t, float b, float l, float r): top{t}, bottom{b}, left{l}, right{r} {}
	UIPadding(float padding): top{padding}, bottom{padding}, left{padding}, right{padding} {}

	float top, bottom;
	float left, right;
};

struct UIRenderCommand {
	uint32_t id;
	uint32_t zindex;
	BoundingBox boundingBox;
	glm::vec4 backgroundColor;
	std::string text;
	UIRenderCommandType commandType;
};

struct UILayoutElement {
	std::string id;

	float x;
	float y;
	UIAxisSize width;
	UIAxisSize height;

	UILayoutDirection layoutDirection;
	glm::vec4 backgroundColor;

	UIPadding padding;
	float childGap;

	bool isText{ false }; // TODO: REMOVE THIS
	std::string text;
	uint32_t parent;
	std::vector<uint32_t> children; // reference to context->layoutElementChildrenIndices
};

struct UIElementOptions {
	uint32_t id;

	UIAxisSize width;
	UIAxisSize height;

	UILayoutDirection layoutDirection{};
	glm::vec4 backgroundColor { 0.0f, 0.0f, 0.0f, 0.0f };

	UIPadding padding;
	float childGap;

	std::string_view text;
};

struct UIElement {
	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
};

struct UITextElement {
	BoundingBox boundingBox;
	glm::vec4 backgroundColor;

	std::string text;

	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
};


}// namespace pm
