#include "ui_manager.h"
#include <iostream>
#include <format>
#include "ui/ui_types.h"
#include "utils/fonts.h"
#include <algorithm>
#include <iterator>
#include <ranges>

namespace pm::UI {

void initRenderContext(InitRenderContextOptions options) {
	uiContext = new UIContext();

	uiContext->windowWidth = options.width;
	uiContext->windowHeight = options.height;
}

void cleanupRenderContext() {
	delete uiContext;
}

void clearContext() {
	uiContext->renderCommands.clear();
	uiContext->layoutElements.clear();
	uiContext->openLayoutElements = {};
	uiContext->layoutElementChildrenIndices.clear();
}

UIContext* getUIContext() {
	return uiContext;
}

void onResizeCallback(float width, float height) {
	auto context = getUIContext();
	if (!context) {
		return;
	}

	context->windowWidth = width;
	context->windowHeight = height;
}

void setPointerState(float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown) {
	auto context = getUIContext();

	context->hoveredIds.clear();

	context->pointerState.x = mouseX;
	context->pointerState.y = mouseY;
	context->pointerState.xRel = relMouseX;
	context->pointerState.yRel = relMouseY;

	auto& clickState = context->pointerState.pointerClickState;
	if (isPointerDown) {
		if (clickState == PointerClickState::PRESSED_THIS_FRAME) {
			context->pointerState.pointerClickState = PointerClickState::PRESSED;
			context->interactionState.isDragging = true;
		} else if (clickState != PointerClickState::PRESSED) {
			context->pointerState.pointerClickState = PointerClickState::PRESSED_THIS_FRAME;
		}
	} else {
		if (clickState == PointerClickState::RELEASED_THIS_FRAME) {
			context->pointerState.pointerClickState = PointerClickState::RELEASED;
		} else if (clickState != PointerClickState::RELEASED) {
			context->pointerState.pointerClickState = PointerClickState::RELEASED_THIS_FRAME;
			context->interactionState.isDragging = false;
			context->interactionState.elementId = 0;
		}
	}

	// We record the first "click" inside a bounding box.
	// The point of this is to not bubble up the events up the hierarchy (at least for now, we could add the option to do it).
	// We iterate the elements in reverse to go up the tree starting from the deepest children
	// TODO(piero): Probably need to rework this logic when adding floating elements.
	bool firstEvent{ true };
	for (auto& element : context->layoutElements | std::ranges::views::reverse) {
		auto bb = BoundingBox{ .x = element.x, .y = element.y, .width = element.width.size, .height = element.height.size };
		if (isInsideBoundingBox(mouseX, mouseY, bb)) {
			// Don't process hover callbacks if we are dragging the mouse around.
			// TODO(piero): Do we actually want this? Maybe make it an option.
			if (element.onHoverCallback && !context->interactionState.isDragging) {
				element.onHoverCallback(element.id, context->pointerState);
			}

			// When we click inside an element, record the interaction
			if (context->pointerState.pointerClickState == PointerClickState::PRESSED_THIS_FRAME && firstEvent) {
				std::cout << std::format("{} {} {} {}\n", bb.x, bb.y, bb.width, bb.height);
				context->interactionState.elementId = element.id;
				firstEvent = false;
			}

			context->hoveredIds.push_back(element.id);
		}
	}

	// Value dragging
	if (context->pointerState.pointerClickState == PointerClickState::PRESSED && context->interactionState.isDragging) {
		auto& element = context->layoutElements.at(context->interactionState.elementId);
		switch (element.data.dataType) {
		case INT: {
			if (element.data.valueInt) {
				*element.data.valueInt = std::clamp(static_cast<int>(*element.data.valueInt + context->pointerState.xRel * 0.01f), element.data.minInt, element.data.maxInt);
			}
			break;
		}
		case FLOAT: {
			if (element.data.valueFloat) {
				*element.data.valueFloat = std::clamp(*element.data.valueFloat + context->pointerState.xRel * 0.01f, element.data.minFloat, element.data.maxFloat);
			}
			break;
		}
		case DOUBLE: {
			if (element.data.valueDouble) {
				*element.data.valueDouble = std::clamp(*element.data.valueDouble + context->pointerState.xRel * 0.01, element.data.minDouble, element.data.maxDouble);
			}
			break;
		}
		default: {
			break;
		}
		}
	}
}

BoundingBox getTextSize() {
	return {};
}

void beginLayout() {
	clearContext();// TODO: remove when refactoring to use temporary arenas
	auto context = getUIContext();
	// TODO: init temp memory arena?

	// create root element
	openElement();

	// Configure root element
	auto& rootElement = context->layoutElements.back();
	rootElement.width.size = context->windowWidth;
	rootElement.height.size = context->windowHeight;
	rootElement.layoutDirection = UILayoutDirection::VERTICAL;
	rootElement.childGap = 20.0f;
}

std::vector<UIRenderCommand> endLayout() {
	auto context = getUIContext();
	closeElement();

	calculateFinalLayout();

	return context->renderCommands;
}

void openElement() {
	auto context = getUIContext();

	// Create new layout for the element
	context->layoutElements.emplace_back();
	context->openLayoutElements.push(context->layoutElements.size() - 1);
	context->layoutElementChildrenIndices.emplace_back(context->layoutElements.size() - 1);

	// Assign id to element
	auto& element = context->layoutElements.back();
	element.id = context->layoutElements.size() - 1;
}

void closeElement() {
	auto context = getUIContext();

	auto closedElementIndex = std::move(context->openLayoutElements.top());
	context->openLayoutElements.pop();

	auto& openLayoutElement = context->layoutElements.at(closedElementIndex);

	// Set parent to the open layout element
	if (!context->openLayoutElements.empty()) {
		uint32_t parentIndex = context->openLayoutElements.top();
		openLayoutElement.parent = parentIndex;
	}

	// add parent padding
	if (openLayoutElement.parent >= 0) {
		auto& parent = context->layoutElements.at(openLayoutElement.parent);
		openLayoutElement.x += parent.padding.left;
		openLayoutElement.y += parent.padding.top;
	}

	// add children indices to closing layout element
	// Find the starting index in the children array for the open layout index we are currently closing
	auto beginIndex = std::find(context->layoutElementChildrenIndices.begin(), context->layoutElementChildrenIndices.end(), closedElementIndex) - context->layoutElementChildrenIndices.begin();
	if ((beginIndex + 1) < context->layoutElementChildrenIndices.size()) {
		std::ranges::copy(context->layoutElementChildrenIndices.begin() + beginIndex + 1, context->layoutElementChildrenIndices.end(), std::back_inserter(openLayoutElement.children));
		// remove the copied indices
		context->layoutElementChildrenIndices.erase(context->layoutElementChildrenIndices.begin() + beginIndex + 1, context->layoutElementChildrenIndices.end());
	}

	float horizontalPadding = openLayoutElement.padding.left + openLayoutElement.padding.right;
	float verticalPadding = openLayoutElement.padding.top + openLayoutElement.padding.bottom;

	// Iterate children and calculate closing element's Width and Height
	// If we are not a STATIC sized' object, we will FIT the contents of each object.
	// TODO: Handle GROW sizing in (probably) a separate pass.
	if (openLayoutElement.layoutDirection == UILayoutDirection::HORIZONTAL) {
		float leftOffset{ 0.0f };
		if (openLayoutElement.width.sizingMode != UISizingMode::STATIC) {
			openLayoutElement.width.size = horizontalPadding;
		}
		openLayoutElement.width.size += static_cast<float>(openLayoutElement.children.size() - 1) * openLayoutElement.childGap;
		for (auto& childIndex : openLayoutElement.children) {
			auto& child = context->layoutElements.at(childIndex);
			child.x += leftOffset;
			if (openLayoutElement.width.sizingMode != UISizingMode::STATIC) {
				openLayoutElement.width.size += child.width.size;
				openLayoutElement.height.size = std::max(child.height.size + verticalPadding, openLayoutElement.height.size);
			}
			leftOffset += child.width.size + openLayoutElement.childGap;
		}
	} else {
		float topOffset{ 0.0f };
		if (openLayoutElement.height.sizingMode != UISizingMode::STATIC) {
			openLayoutElement.height.size = verticalPadding;
		}
		openLayoutElement.height.size += static_cast<float>(openLayoutElement.children.size() - 1) * openLayoutElement.childGap;
		for (auto& childIndex : openLayoutElement.children) {
			auto& child = context->layoutElements.at(childIndex);
			child.y += topOffset;
			if (openLayoutElement.height.sizingMode != UISizingMode::STATIC) {
				openLayoutElement.width.size = std::max(child.width.size + horizontalPadding, openLayoutElement.width.size);
				openLayoutElement.height.size += child.height.size;
			}
			topOffset += child.height.size + openLayoutElement.childGap;
		}
	}
}

void openTextElement() {
	auto context = getUIContext();

	context->layoutElements.emplace_back();
	context->openLayoutElements.push(context->layoutElements.size() - 1);
	context->layoutElementChildrenIndices.emplace_back(context->layoutElements.size() - 1);

	// Assign id to element
	auto& element = context->layoutElements.back();
	element.id = context->layoutElements.size() - 1;
}

void closeTextElement() {
	auto context = getUIContext();

	auto closedElementIndex = std::move(context->openLayoutElements.top());
	context->openLayoutElements.pop();

	auto& openLayoutElement = context->layoutElements.at(closedElementIndex);

	// Set parent to the open layout element
	if (!context->openLayoutElements.empty()) {
		uint32_t parentIndex = context->openLayoutElements.top();
		openLayoutElement.parent = parentIndex;
	}

	// add parent padding
	if (openLayoutElement.parent >= 0) {
		auto& parent = context->layoutElements.at(openLayoutElement.parent);
		openLayoutElement.x += parent.padding.left;
		openLayoutElement.y += parent.padding.top;
	}

	float horizontalPadding = openLayoutElement.padding.left + openLayoutElement.padding.right;
	float verticalPadding = openLayoutElement.padding.top + openLayoutElement.padding.bottom;

	// Calculate closing element's Width and Height
	if (openLayoutElement.layoutDirection == UILayoutDirection::HORIZONTAL) {
		openLayoutElement.width.size += horizontalPadding;
	} else {
		openLayoutElement.height.size += verticalPadding;
	}

	// TODO: handle text wrapping and truncation
}

void closeCircleElement() {
	auto context = getUIContext();

	auto closedElementIndex = std::move(context->openLayoutElements.top());
	context->openLayoutElements.pop();

	auto& openLayoutElement = context->layoutElements.at(closedElementIndex);

	// Set parent to the open layout element
	if (!context->openLayoutElements.empty()) {
		uint32_t parentIndex = context->openLayoutElements.top();
		openLayoutElement.parent = parentIndex;
	}

	// add parent padding
	if (openLayoutElement.parent >= 0) {
		auto& parent = context->layoutElements.at(openLayoutElement.parent);
		openLayoutElement.x += parent.padding.left;
		openLayoutElement.y += parent.padding.top;
	}

	float horizontalPadding = openLayoutElement.padding.left + openLayoutElement.padding.right;
	float verticalPadding = openLayoutElement.padding.top + openLayoutElement.padding.bottom;

	// Calculate closing element's Width and Height
	if (openLayoutElement.layoutDirection == UILayoutDirection::HORIZONTAL) {
		openLayoutElement.width.size += horizontalPadding;
	} else {
		openLayoutElement.height.size += verticalPadding;
	}
}

// DFS to add parent position to children
void computeFinalSizes() {
	auto context = getUIContext();

	std::stack<uint32_t> stack1;

	stack1.push(0);

	while (!stack1.empty()) {
		uint32_t index = stack1.top();
		stack1.pop();

		auto& layoutElement = context->layoutElements.at(index);
		auto& parentElement = context->layoutElements.at(layoutElement.parent);

		// Element positions are relative. Before rendering we need to add the parent's position.
		layoutElement.x += parentElement.x;
		layoutElement.y += parentElement.y;

		for (auto& childIndex : layoutElement.children) {
			stack1.push(childIndex);
		}
	}
}

void calculateFinalLayout() {
	auto context = getUIContext();

	computeFinalSizes();

	std::stack<uint32_t> indices{};
	indices.push(0);// root element

	while (!indices.empty()) {
		auto index = std::move(indices.top());
		indices.pop();

		auto& layoutElement = context->layoutElements.at(index);

		UIRenderCommand c{
			.id = index,
			.zindex = 0,
			.boundingBox = {
				.x = layoutElement.x,
				.y = layoutElement.y,
				.width = layoutElement.width.size,
				.height = layoutElement.height.size },
			.backgroundColor = layoutElement.backgroundColor,
			.commandType = UIRenderCommandType::RECTANGLE
		};

		if (layoutElement.isCircle) {
			c.commandType = UIRenderCommandType::CIRCLE;
			c.circleType = layoutElement.circleType;
			c.thickness = layoutElement.thickness;
			c.radius = layoutElement.radius;
			c.segments = layoutElement.segments;
		} else if (layoutElement.isText) {
			c.commandType = UIRenderCommandType::TEXT;
			c.text = layoutElement.text;
		}

		context->renderCommands.push_back(c);

		for (auto& childIndex : layoutElement.children) {
			indices.push(childIndex);
		}
	}
}

void setFont(FontAsset* font) {
	auto context = getUIContext();

	context->fontAsset = font;
}

void pushText(UIElementOptions options) {
	auto context = getUIContext();

	float width{}, height{};

	getTextDimensions(options.text, context->fontAsset, width, height);
	// std::cout << std::format("text size: {}x{}\n", width, height);

	auto& layoutElement = context->layoutElements.back();
	layoutElement.isText = true;
	layoutElement.width.size = width;
	layoutElement.height.size = height;
	layoutElement.text = options.text;

	layoutElement.onHoverCallback = options.onHoverCallback;
	layoutElement.onClickCallback = options.onClickCallback;

	layoutElement.data = options.data;
}

void pushBox(UIElementOptions options) {
	auto context = getUIContext();
	auto& layoutElement = context->layoutElements.back();
	layoutElement.width = options.width;
	layoutElement.height = options.height;
	layoutElement.layoutDirection = options.layoutDirection;
	layoutElement.backgroundColor = options.backgroundColor;
	layoutElement.padding = options.padding;
	layoutElement.childGap = options.childGap;
	layoutElement.onHoverCallback = options.onHoverCallback;
	layoutElement.onClickCallback = options.onClickCallback;
}

void getTextDimensions(std::string_view text, FontAsset* font, float& width, float& height) {
	auto [w, h] = generateTextFromFont(text, 16.0f, font);
	width = w;
	height = h;
}

bool isHovered() {
	auto context = getUIContext();
	auto openElementIndex = context->openLayoutElements.top();
	auto& openLayoutElement = context->layoutElements.at(openElementIndex);

	// Don't process hover while dragging
	// TODO(piero): There is a "possible" bug here. We probably want to still process
	// hover events on the item we clicked on while we drag?
	if (context->interactionState.isDragging) {
		return false;
	}

	for (auto& hoveredId : context->hoveredIds) {
		if (hoveredId == openLayoutElement.id) {
			return true;
		}
	}

	return false;
}

bool isInsideBoundingBox(float x, float y, BoundingBox bb) {
	return x >= bb.x && x <= bb.x + bb.width && y >= bb.y && y <= bb.y + bb.height;
}

void pushCircle(float radius, uint32_t segments, float thickness, glm::vec4 color) {
	openElement();
	auto context = getUIContext();
	auto& layoutElement = context->layoutElements.back();

	layoutElement.width.size = radius * 2;
	layoutElement.height.size = radius * 2;
	layoutElement.isCircle = true;
	layoutElement.radius = radius;
	layoutElement.segments = segments;
	layoutElement.thickness = thickness;
	layoutElement.backgroundColor = color;
	layoutElement.circleType = CircleType::OUTLINE;

	closeCircleElement();
}

void pushCircleFilled(float radius, uint32_t segments, glm::vec4 color) {
	openElement();
	auto context = getUIContext();
	auto& layoutElement = context->layoutElements.back();

	layoutElement.width.size = radius * 2;
	layoutElement.height.size = radius * 2;
	layoutElement.isCircle = true;
	layoutElement.radius = radius;
	layoutElement.segments = segments;
	layoutElement.backgroundColor = color;
	layoutElement.circleType = CircleType::FILLED;

	closeCircleElement();
}

}// namespace pm::UI
