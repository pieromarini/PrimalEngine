#pragma once

#include "utils/fonts.h"
#include "ui_types.h"
#include "utils/geometry.h"
#include "memory/data_structures/fixed_array.h"
#include "memory/arena.h"
#include "vk_types.h"

namespace pm::UI {

// TODO(piero): I think this is OK for now. I don't think we will have extremely nested/complicated UI layouts that could exceed this.
// This is used to allocate FixedArrays
constexpr uint32_t maxElementCount = 8192;

struct UIContext {
	FixedArray<UIRenderCommand> renderCommands;

	FixedArray<UILayoutElement> layoutElements;
	FixedArray<UILayoutElementData> layoutElementsData;
	FixedArray<uint32_t> layoutElementChildrenIndices;

	FixedArray<uint32_t> openLayoutElements; // elements with an open Layout

	// Interactions
	PointerState pointerState;
	InteractionState interactionState;
	FixedArray<uint32_t> hoveredIds;

	// Fonts
	FontAsset* fontAsset{};

	// Window context
	float windowWidth{}, windowHeight{};

	// Window target for this layout
	PrimalWindow* window{};

	MemoryArena* arena{};
	MemoryArena tempArena{};
};

static UIContext* uiContext;

void initRenderContext(MemoryArena* arena);
void cleanupRenderContext();

UIContext* getUIContext();

void onResizeCallback(float width, float height);
void setPointerState(float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown);

void clearContext();

PrimalWindow* createWindow(std::string_view name, int32_t width, int32_t height);
void beginWindow(PrimalWindow* window);
void endWindow();
void beginLayout();
FixedArray<UIRenderCommand> endLayout();

void openElement();
void closeElement();

void openTextElement();
void closeTextElement();
void closeCircleElement();
void closeViewportElement();
void closeDockSpaceElement();

// Utils
void getTextDimensions(PrimalString& text, FontAsset* font, float& width, float& height);

// Layout
void computeFinalSizes();
void calculateFinalLayout();

// Base layout API
void setFont(FontAsset* font);

void pushText(UIElementOptions options);
void pushBox(UIElementOptions options);
void pushCircle(float radius, uint32_t segments, float thickness, glm::vec4 color);
void pushCircleFilled(float radius, uint32_t segments, glm::vec4 color);

void pushPanel(UIElementOptions options);
void pushTitleBar(UIElementOptions options);
void pushDockSpace(UIElementOptions options);

// Interactions
bool isHovered();
bool isInsideBoundingBox(float x, float y, BoundingRect bb);

}// namespace pm::UI
