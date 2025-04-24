#pragma once

#include "memory/data_structures/fixed_array.h"
#include "window.h"
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>

#include "memory/data_structures/primal_string.h"

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
	CIRCLE,
	TEXT,
	VIEWPORT,
	PANEL,
	TITLEBAR,
	DOCKSPACE
};

struct UIAxisSize {
	float size;
	UISizingMode sizingMode;
};

struct BoundingRect {
	float x, y;
	float width, height;
};

struct UIVertex {
	glm::vec3 position;
	float uv_x;
	glm::vec3 color;
	float uv_y;
};

enum UIDataType {
	INT,
	FLOAT,
	DOUBLE,
	STRING
};

enum class CircleType {
	FILLED,
	OUTLINE
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

	BoundingRect boundingRect;

	glm::vec4 backgroundColor;

	// text
	PrimalString text;

	// circle
	float radius;
	float thickness;
	uint32_t segments;
	CircleType circleType;

	// viewport
	uint32_t textureId;

	UIRenderCommandType commandType;
};

enum PointerClickState {
	PRESSED,
	RELEASED,
	PRESSED_THIS_FRAME,
	RELEASED_THIS_FRAME
};

struct PointerState {
	float x{};
	float y{};

	float xRel{};
	float yRel{};

	PointerClickState pointerClickState{ PointerClickState::RELEASED };
	bool isDragging{ false };
};

struct InteractionState {
	uint32_t elementId{}; // element being interacted with
	bool isDragging{ false };
};

struct UILayoutElementData {
	UIDataType dataType;
	union {
		double* valueDouble;
		float* valueFloat;
		int* valueInt;
	};
	union {
		double minDouble;
		float minFloat;
		int minInt;
	};
	union {
		double maxDouble;
		float maxFloat;
		int maxInt;
	};
};

using InteractionCallbackSignature = void(uint32_t elementId, PointerState pointerState);

enum UILayoutElementType {
	RECT_ELEMENT,
	TEXT_ELEMENT,
	CIRCLE_ELEMENT,
	VIEWPORT_ELEMENT,
	PANEL_ELEMENT,
	TITLEBAR_ELEMENT,
	DOCKSPACE_ELEMENT
};

struct UILayoutElement {
	uint32_t id;

	float x;
	float y;
	UIAxisSize width;
	UIAxisSize height;

	UILayoutDirection layoutDirection;
	glm::vec4 backgroundColor;

	UIPadding padding;
	float childGap;

	std::function<InteractionCallbackSignature> onHoverCallback;
	std::function<InteractionCallbackSignature> onClickCallback;

	UILayoutElementType type;

	uint32_t parent;
	FixedArray<uint32_t> children; // reference to context->layoutElementChildrenIndices

	// text
	PrimalString text;

	// circle
	float radius;
	uint32_t segments;
	float thickness;
	CircleType circleType;

	// viewport
	uint32_t textureId;

	UILayoutElementData data;
};

struct UIElementOptions {
	uint32_t id;

	UIAxisSize width;
	UIAxisSize height;

	UILayoutDirection layoutDirection{};
	glm::vec4 backgroundColor { 0.0f, 0.0f, 0.0f, 0.0f };

	UIPadding padding;
	float childGap;

	std::function<InteractionCallbackSignature> onHoverCallback;
	std::function<InteractionCallbackSignature> onClickCallback;

	// text
	std::string text;

	// circle
	float radius;
	uint32_t segments;
	float thickness;

	// viewport
	uint32_t textureId;

	UILayoutElementData data;
};

struct UIElement {
	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
};

struct UITextElement {
	BoundingRect boundingBox;
	glm::vec4 backgroundColor;

	std::string text;

	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
};


}// namespace pm
