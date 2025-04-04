#include "ui_manager.h"
#include "ui/ui_types.h"
#include "utils/fonts.h"
#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <queue>
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
	uiContext->uiElements.clear();
	uiContext->textElements.clear();
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
	rootElement.id = "UI_ROOT";
	rootElement.width = context->windowWidth;
	rootElement.height = context->windowHeight;
	rootElement.layoutDirection = UILayoutDirection::VERTICAL;
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
	if (openLayoutElement.parent) {
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
		if (openLayoutElement.sizingMode != UISizingMode::STATIC) {
			openLayoutElement.width = horizontalPadding;
			openLayoutElement.width += static_cast<float>(openLayoutElement.children.size() - 1) * openLayoutElement.childGap;
		}
		for (auto& childIndex : openLayoutElement.children) {
			auto& child = context->layoutElements.at(childIndex);
			child.x += leftOffset;
			if (openLayoutElement.sizingMode != UISizingMode::STATIC) {
				openLayoutElement.width += child.width;
				openLayoutElement.height = std::max(child.height + verticalPadding, openLayoutElement.height);
			}
			leftOffset += child.width + openLayoutElement.childGap;
		}
	} else {
		float topOffset{ 0.0f };
		if (openLayoutElement.sizingMode != UISizingMode::STATIC) {
			openLayoutElement.height = verticalPadding;
			openLayoutElement.height += static_cast<float>(openLayoutElement.children.size() - 1) * openLayoutElement.childGap;
		}
		for (auto& childIndex : openLayoutElement.children) {
			auto& child = context->layoutElements.at(childIndex);
			child.y += topOffset;
			if (openLayoutElement.sizingMode != UISizingMode::STATIC) {
				openLayoutElement.width = std::max(child.width + horizontalPadding, openLayoutElement.width);
				openLayoutElement.height += child.height;
			}
			topOffset += child.height + openLayoutElement.childGap;
		}
	}
}

void openTextElement() {
	auto context = getUIContext();

	context->layoutElements.emplace_back();
	context->openLayoutElements.push(context->layoutElements.size() - 1);
	context->layoutElementChildrenIndices.emplace_back(context->layoutElements.size() - 1);
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
	if (openLayoutElement.parent) {
		auto& parent = context->layoutElements.at(openLayoutElement.parent);
		openLayoutElement.x += parent.padding.left;
		openLayoutElement.y += parent.padding.top;
	}

	float horizontalPadding = openLayoutElement.padding.left + openLayoutElement.padding.right;
	float verticalPadding = openLayoutElement.padding.top + openLayoutElement.padding.bottom;

	// Calculate closing element's Width and Height
	if (openLayoutElement.layoutDirection == UILayoutDirection::HORIZONTAL) {
		openLayoutElement.width += horizontalPadding;
	} else {
		openLayoutElement.height += verticalPadding;
	}

	// TODO: handle text wrapping and truncation
}

/*
 * This does a post-order traversal of the UI hierarchy
 * to calculate the final sizes for each element, depending on their children.
 * Left-most child gets processed first
 */
void computeFinalSizes() {
	auto context = getUIContext();

	std::stack<uint32_t> stack1;
	std::stack<uint32_t> stack2;

	stack1.push(0);

	while(!stack1.empty()) {
		uint32_t index = stack1.top();
		stack1.pop();
		auto& layoutElement = context->layoutElements.at(index);

		stack2.push(index);

		for (auto& childIndex : layoutElement.children) {
			stack1.push(childIndex);
		}
	}

	float leftOffset{ 0.0f };
	float topOffset{ 0.0f };

	while(!stack2.empty()) {
		uint32_t index = stack2.top();
		stack2.pop();
		auto& layoutElement = context->layoutElements.at(index);
		auto& parentElement = context->layoutElements.at(layoutElement.parent);
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
		auto& parentElement = context->layoutElements.at(layoutElement.parent);

		UIRenderCommand c{
			.id = index,
			.zindex = 0,
			.boundingBox = {
				.x = parentElement.x + layoutElement.x,
				.y = parentElement.y + layoutElement.y,
				.width = layoutElement.width,
				.height = layoutElement.height },
			.backgroundColor = layoutElement.backgroundColor,
			.commandType = layoutElement.isText ? UIRenderCommandType::TEXT : UIRenderCommandType::RECTANGLE
		};

		if (layoutElement.isText) {
			c.text = layoutElement.text;
		}

		context->renderCommands.push_back(c);

		for (auto& childIndex : layoutElement.children) {
			indices.push(childIndex);
		}
	}
}

void setFont(FontInfo fontInfo) {
	auto context = getUIContext();

	context->fontInfo = fontInfo;
}

void pushText(UIElementOptions options) {
	auto context = getUIContext();

	float width{}, height{};

	getTextDimensions(options.text, context->fontInfo, width, height);
	// std::cout << std::format("text: {}x{}\n", width, height);

	auto& layoutElement = context->layoutElements.back();
	layoutElement.isText = true;
	layoutElement.width = width;
	layoutElement.height = height;
	layoutElement.text = options.text;
}

void pushBox(UIElementOptions options) {
	auto context = getUIContext();
	auto& layoutElement = context->layoutElements.back();
	layoutElement.width = options.width;
	layoutElement.height = options.height;
	layoutElement.layoutDirection = options.layoutDirection;
	layoutElement.sizingMode = options.sizingMode;
	layoutElement.backgroundColor = options.backgroundColor;
	layoutElement.padding = options.padding;
	layoutElement.childGap = options.childGap;
}

void pushTriangle(UIElementOptions options) {
	auto context = getUIContext();
}

void getTextDimensions(std::string_view text, FontInfo& fontInfo, float& width, float& height) {
	auto [w, h] = generateTextFromFont(text, fontInfo.textureWidth, fontInfo.fontChars);
	width = w;
	height = h;
}

}// namespace pm::UI
