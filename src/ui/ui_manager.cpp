#include "ui_manager.h"
#include <chrono>
#include <iostream>
#include <format>
#include "memory/arena.h"
#include "memory/data_structures/fixed_array.h"
#include "ui/ui_types.h"
#include "utils/fonts.h"

namespace pm::UI {

void initRenderContext(MemoryArena* arena, InitRenderContextOptions options) {
	uiContext = MemoryArenaPush(UIContext, 1, arena);

	uiContext->arena = arena;

	uiContext->windowWidth = options.width;
	uiContext->windowHeight = options.height;
	// Persistent data
	uiContext->hoveredIds = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, arena);

	// Ephemeral data. Reset each frame.
	uiContext->tempArena = MemoryArena_create(MEGABYTE(10));
	MemoryArena_commit(&uiContext->tempArena, uiContext->tempArena.size);

	auto tempArena = &uiContext->tempArena;

	uiContext->layoutElements = MemoryArenaCreateArray(FixedArray<UILayoutElement>, UILayoutElement, maxElementCount, tempArena);
	uiContext->layoutElementsData = MemoryArenaCreateArray(FixedArray<UILayoutElementData>, UILayoutElementData, maxElementCount, tempArena);
	uiContext->layoutElementChildrenIndices = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, tempArena);
	uiContext->openLayoutElements = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, tempArena);
	uiContext->renderCommands = MemoryArenaCreateArray(FixedArray<UIRenderCommand>, UIRenderCommand, maxElementCount, tempArena);
}

void cleanupRenderContext() {
	auto context = getUIContext();
	MemoryArena_destroy(&context->tempArena);
	MemoryArena_destroy(context->arena);
}

void clearContext() {
	auto context = getUIContext();

	// Clear temp arena and re-init per-frame arrays
	auto tempArena = &context->tempArena;
	MemoryArena_clear(tempArena);

	uiContext->layoutElements = MemoryArenaCreateArray(FixedArray<UILayoutElement>, UILayoutElement, maxElementCount, tempArena);
	uiContext->layoutElementsData = MemoryArenaCreateArray(FixedArray<UILayoutElementData>, UILayoutElementData, maxElementCount, tempArena);
	uiContext->layoutElementChildrenIndices = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, tempArena);
	uiContext->openLayoutElements = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, tempArena);
	uiContext->renderCommands = MemoryArenaCreateArray(FixedArray<UIRenderCommand>, UIRenderCommand, maxElementCount, tempArena);
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

	FixedArray_clear(context->hoveredIds);

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
	for (int32_t i = (int)context->layoutElements.length - 1; i >= 0; --i) {
		auto element = FixedArray_get(context->layoutElements, i);
		auto bb = BoundingRect{ .x = element->x, .y = element->y, .width = element->width.size, .height = element->height.size };
		if (isInsideBoundingBox(mouseX, mouseY, bb)) {
			// Don't process hover callbacks if we are dragging the mouse around.
			// TODO(piero): Do we actually want this? Maybe make it an option.
			if (element->onHoverCallback && !context->interactionState.isDragging) {
				element->onHoverCallback(element->id, context->pointerState);
			}

			// When we click inside an element, record the interaction
			if (context->pointerState.pointerClickState == PointerClickState::PRESSED_THIS_FRAME && firstEvent) {
				// std::cout << std::format("{} {} {} {}\n", bb.x, bb.y, bb.width, bb.height);
				context->interactionState.elementId = element->id;
				firstEvent = false;
			}

			FixedArray_add(context->hoveredIds, element->id);
		}
	}

	// Value dragging
	if (context->pointerState.pointerClickState == PointerClickState::PRESSED && context->interactionState.isDragging) {
		auto element = FixedArray_get(context->layoutElements, context->interactionState.elementId);
		switch (element->data.dataType) {
		case INT: {
			if (element->data.valueInt) {
				*element->data.valueInt = std::min(static_cast<int>(*element->data.valueInt + context->pointerState.xRel * 0.01f), std::max(element->data.minInt, element->data.maxInt));
			}
			break;
		}
		case FLOAT: {
			if (element->data.valueFloat) {
				*element->data.valueFloat = std::min(*element->data.valueFloat + context->pointerState.xRel * 0.01f, std::max(element->data.minFloat, element->data.maxFloat));
			}
			break;
		}
		case DOUBLE: {
			if (element->data.valueDouble) {
				*element->data.valueDouble = std::min(*element->data.valueDouble + context->pointerState.xRel * 0.01, std::max(element->data.minDouble, element->data.maxDouble));
			}
			break;
		}
		default: {
			break;
		}
		}
	}
}

void beginLayout() {
	clearContext();
	auto context = getUIContext();

	// create root element
	openElement();

	// Configure root element
	auto rootElement = FixedArray_back(context->layoutElements);
	rootElement->width.size = context->windowWidth;
	rootElement->height.size = context->windowHeight;
	rootElement->layoutDirection = UILayoutDirection::VERTICAL;
	rootElement->childGap = 0.0f;
}

FixedArray<UIRenderCommand> endLayout() {
	auto context = getUIContext();
	closeElement();

	calculateFinalLayout();

	return context->renderCommands;
}

void openElement() {
	auto context = getUIContext();

	// Create new layout for the element
	FixedArray_add(context->layoutElements, { .id = context->layoutElements.length, .children = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, &context->tempArena) });
	FixedArray_add(context->openLayoutElements, context->layoutElements.length - 1);
	FixedArray_add(context->layoutElementChildrenIndices, context->layoutElements.length - 1);
}

void closeElement() {
	auto context = getUIContext();

	auto closedElementIndex = *FixedArray_top(context->openLayoutElements);
	FixedArray_pop(context->openLayoutElements);

	auto openLayoutElement = FixedArray_get(context->layoutElements, closedElementIndex);

	// Set parent to the open layout element
	if (!FixedArray_empty(context->openLayoutElements)) {
		auto parentIndex = FixedArray_top(context->openLayoutElements);
		openLayoutElement->parent = *parentIndex;
	}

	// add parent padding
	if (openLayoutElement->parent >= 0) {
		auto parent = FixedArray_get(context->layoutElements, openLayoutElement->parent);
		openLayoutElement->x += parent->padding.left;
		openLayoutElement->y += parent->padding.top;
	}

	// add children indices to closing layout element
	// Find the starting index in the children array for the open layout index we are currently closing
	auto beginIndex = FixedArray_findIndex(context->layoutElementChildrenIndices, closedElementIndex);
	if ((beginIndex != -1) && ((beginIndex + 1) < context->layoutElementChildrenIndices.length)) {

		// copy indices
		for (uint32_t i = beginIndex + 1; i < context->layoutElementChildrenIndices.length; ++i) {
			auto t = FixedArray_getValue(context->layoutElementChildrenIndices, i);
			FixedArray_add(openLayoutElement->children, t);
		}

		// remove copied indices
		FixedArray_removeRange(context->layoutElementChildrenIndices, beginIndex + 1, context->layoutElementChildrenIndices.length);
	}

	float horizontalPadding = openLayoutElement->padding.left + openLayoutElement->padding.right;
	float verticalPadding = openLayoutElement->padding.top + openLayoutElement->padding.bottom;

	// Iterate children and calculate closing element's Width and Height
	// If we are not a STATIC sized' object, we will FIT the contents of each object.
	// TODO: Handle GROW sizing in (probably) a separate pass.
	if (openLayoutElement->layoutDirection == UILayoutDirection::HORIZONTAL) {
		float leftOffset{ 0.0f };
		if (openLayoutElement->width.sizingMode != UISizingMode::STATIC) {
			openLayoutElement->width.size = horizontalPadding;
		}
		openLayoutElement->width.size += static_cast<float>(openLayoutElement->children.length - 1) * openLayoutElement->childGap;
		for (uint32_t i = 0; i < openLayoutElement->children.length; ++i) {
			auto childIndex = FixedArray_getValue(openLayoutElement->children, i);
			auto child = FixedArray_get(context->layoutElements, childIndex);
			child->x += leftOffset;
			if (openLayoutElement->width.sizingMode != UISizingMode::STATIC) {
				openLayoutElement->width.size += child->width.size;
				openLayoutElement->height.size = std::max(child->height.size + verticalPadding, openLayoutElement->height.size);
			}
			leftOffset += child->width.size + openLayoutElement->childGap;
		}
	} else {
		float topOffset{ 0.0f };
		if (openLayoutElement->height.sizingMode != UISizingMode::STATIC) {
			openLayoutElement->height.size = verticalPadding;
		}
		openLayoutElement->height.size += static_cast<float>(openLayoutElement->children.length - 1) * openLayoutElement->childGap;
		for (uint32_t i = 0; i < openLayoutElement->children.length; ++i) {
			auto childIndex = FixedArray_getValue(openLayoutElement->children, i);
			auto child = FixedArray_get(context->layoutElements, childIndex);
			child->y += topOffset;
			if (openLayoutElement->height.sizingMode != UISizingMode::STATIC) {
				openLayoutElement->width.size = std::max(child->width.size + horizontalPadding, openLayoutElement->width.size);
				openLayoutElement->height.size += child->height.size;
			}
			topOffset += child->height.size + openLayoutElement->childGap;
		}
	}
}

void openTextElement() {
	auto context = getUIContext();

	FixedArray_add(context->layoutElements, { .id = context->layoutElements.length });
	FixedArray_add(context->openLayoutElements, context->layoutElements.length - 1);
	FixedArray_add(context->layoutElementChildrenIndices, context->layoutElements.length - 1);
}

void closeTextElement() {
	auto context = getUIContext();

	auto closedElementIndex = *FixedArray_top(context->openLayoutElements);
	FixedArray_pop(context->openLayoutElements);

	auto openLayoutElement = FixedArray_get(context->layoutElements, closedElementIndex);

	// Set parent to the open layout element
	if (!FixedArray_empty(context->openLayoutElements)) {
		auto parentIndex = FixedArray_top(context->openLayoutElements);
		openLayoutElement->parent = *parentIndex;
	}

	// add parent padding
	if (openLayoutElement->parent >= 0) {
		auto parent = FixedArray_get(context->layoutElements, openLayoutElement->parent);
		openLayoutElement->x += parent->padding.left;
		openLayoutElement->y += parent->padding.top;
	}

	float horizontalPadding = openLayoutElement->padding.left + openLayoutElement->padding.right;
	float verticalPadding = openLayoutElement->padding.top + openLayoutElement->padding.bottom;

	// Calculate closing element's Width and Height
	if (openLayoutElement->layoutDirection == UILayoutDirection::HORIZONTAL) {
		openLayoutElement->width.size += horizontalPadding;
	} else {
		openLayoutElement->height.size += verticalPadding;
	}

	// TODO: handle text wrapping and truncation
}

void closeCircleElement() {
	auto context = getUIContext();

	auto closedElementIndex = *FixedArray_top(context->openLayoutElements);
	FixedArray_pop(context->openLayoutElements);

	auto openLayoutElement = FixedArray_get(context->layoutElements, closedElementIndex);

	// Set parent to the open layout element
	if (!FixedArray_empty(context->openLayoutElements)) {
		auto parentIndex = FixedArray_top(context->openLayoutElements);
		openLayoutElement->parent = *parentIndex;
	}

	// add parent padding
	if (openLayoutElement->parent >= 0) {
		auto parent = FixedArray_get(context->layoutElements, openLayoutElement->parent);
		openLayoutElement->x += parent->padding.left;
		openLayoutElement->y += parent->padding.top;
	}

	float horizontalPadding = openLayoutElement->padding.left + openLayoutElement->padding.right;
	float verticalPadding = openLayoutElement->padding.top + openLayoutElement->padding.bottom;

	// Calculate closing element's Width and Height
	if (openLayoutElement->layoutDirection == UILayoutDirection::HORIZONTAL) {
		openLayoutElement->width.size += horizontalPadding;
	} else {
		openLayoutElement->height.size += verticalPadding;
	}
}

// DFS to add parent position to children
void computeFinalSizes() {
	auto context = getUIContext();

	auto stack = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, &uiContext->tempArena);
	FixedArray_add(stack, static_cast<uint32_t>(0));

	while (!FixedArray_empty(stack)) {
		auto index = *FixedArray_top(stack);
		FixedArray_pop(stack);

		auto layoutElement = FixedArray_get(context->layoutElements, index);
		auto parentElement = FixedArray_get(context->layoutElements, layoutElement->parent);

		// Element positions are relative. Before rendering we need to add the parent's position.
		layoutElement->x += parentElement->x;
		layoutElement->y += parentElement->y;

		for (uint32_t i = 0; i < layoutElement->children.length; ++i) {
			auto childIndex = FixedArray_getValue(layoutElement->children, i);
			FixedArray_add(stack, childIndex);
		}
	}
}

void calculateFinalLayout() {
	auto context = getUIContext();

	computeFinalSizes();

	auto indices = MemoryArenaCreateArray(FixedArray<uint32_t>, uint32_t, maxElementCount, &uiContext->tempArena);
	FixedArray_add(indices, static_cast<uint32_t>(0));

	while (!FixedArray_empty(indices)) {
		auto index = *FixedArray_top(indices);
		FixedArray_pop(indices);

		auto layoutElement = FixedArray_get(context->layoutElements, index);

		UIRenderCommand c{
			.id = index,
			.zindex = 0,
			.boundingBox = {
				.x = layoutElement->x,
				.y = layoutElement->y,
				.width = layoutElement->width.size,
				.height = layoutElement->height.size },
			.backgroundColor = layoutElement->backgroundColor,
			.commandType = UIRenderCommandType::RECTANGLE
		};

		if (layoutElement->isCircle) {
			c.commandType = UIRenderCommandType::CIRCLE;
			c.circleType = layoutElement->circleType;
			c.thickness = layoutElement->thickness;
			c.radius = layoutElement->radius;
			c.segments = layoutElement->segments;
		} else if (layoutElement->isText) {
			c.commandType = UIRenderCommandType::TEXT;
			c.text = layoutElement->text;
		}

		FixedArray_add(context->renderCommands, c);

		for (uint32_t i = 0; i < layoutElement->children.length; ++i) {
			auto childIndex = FixedArray_getValue(layoutElement->children, i);
			FixedArray_add(indices, childIndex);
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

	auto layoutElement = FixedArray_back(context->layoutElements);
	layoutElement->text = options.text;

	getTextDimensions(layoutElement->text, context->fontAsset, width, height);
	// std::cout << std::format("text size: {}x{}\n", width, height);

	layoutElement->isText = true;
	layoutElement->width.size = width;
	layoutElement->height.size = height;

	layoutElement->onHoverCallback = options.onHoverCallback;
	layoutElement->onClickCallback = options.onClickCallback;

	layoutElement->data = options.data;
}

void pushBox(UIElementOptions options) {
	auto context = getUIContext();
	auto layoutElement = FixedArray_back(context->layoutElements);
	layoutElement->width = options.width;
	layoutElement->height = options.height;
	layoutElement->layoutDirection = options.layoutDirection;
	layoutElement->backgroundColor = options.backgroundColor;
	layoutElement->padding = options.padding;
	layoutElement->childGap = options.childGap;
	layoutElement->onHoverCallback = options.onHoverCallback;
	layoutElement->onClickCallback = options.onClickCallback;
}

void getTextDimensions(PrimalString& text, FontAsset* font, float& width, float& height) {
	auto [w, h] = generateTextFromFont(text, 16.0f, font);
	width = w;
	height = h;
}

bool isHovered() {
	auto context = getUIContext();
	auto openElementIndex = FixedArray_top(context->openLayoutElements);
	auto openLayoutElement = FixedArray_get(context->layoutElements, *openElementIndex);

	// Don't process hover while dragging
	// TODO(piero): There is a "possible" bug here. We probably want to still process
	// hover events on the item we clicked on while we drag?
	if (context->interactionState.isDragging) {
		return false;
	}

	for (uint32_t i = 0; i < context->hoveredIds.length; ++i) {
		auto hoveredId = FixedArray_getValue(context->hoveredIds, i);
		if (hoveredId == openLayoutElement->id) {
			return true;
		}
	}

	return false;
}

bool isInsideBoundingBox(float x, float y, BoundingRect bb) {
	return x >= bb.x && x <= bb.x + bb.width && y >= bb.y && y <= bb.y + bb.height;
}

void pushCircle(float radius, uint32_t segments, float thickness, glm::vec4 color) {
	openElement();
	auto context = getUIContext();
	auto layoutElement = FixedArray_back(context->layoutElements);

	layoutElement->width.size = radius * 2;
	layoutElement->height.size = radius * 2;
	layoutElement->isCircle = true;
	layoutElement->radius = radius;
	layoutElement->segments = segments;
	layoutElement->thickness = thickness;
	layoutElement->backgroundColor = color;
	layoutElement->circleType = CircleType::OUTLINE;

	closeCircleElement();
}

void pushCircleFilled(float radius, uint32_t segments, glm::vec4 color) {
	openElement();
	auto context = getUIContext();
	auto layoutElement = FixedArray_back(context->layoutElements);

	layoutElement->width.size = radius * 2;
	layoutElement->height.size = radius * 2;
	layoutElement->isCircle = true;
	layoutElement->radius = radius;
	layoutElement->segments = segments;
	layoutElement->backgroundColor = color;
	layoutElement->circleType = CircleType::FILLED;

	closeCircleElement();
}

}// namespace pm::UI
