#pragma once

#include "assets/asset.h"
#include "core/data_structures/fixed_array.h"
#include "core/memory/arena.h"
#include "ui_types.h"


namespace pm::UI {

// TODO(piero): I think this is OK for now. I don't think we will have extremely nested/complicated UI layouts that could exceed this.
// This is used to allocate FixedArrays
constexpr uint32_t maxElementCount = 8192;

struct UIContext {
	FixedArray<UIRenderCommand> renderCommands;

	FixedArray<uint32_t> layoutElementChildrenIndices;
	FixedArray<uint32_t> openLayoutElements;// elements with an open Layout

	// Interactions
	PointerState pointerState;
	InteractionState interactionState;
	FixedArray<uint32_t> hoveredIds;

	// window batches
	FixedArray<UIWindowBatchCommands> windowCommands;

	// associates window id with their layout elements.
	FixedArray<FixedArray<UILayoutElement>> layoutElements;

	// Fonts
	FontAsset* fontAsset{};

	// Window context
	float windowWidth{}, windowHeight{};

	// Window target for this layout
	PrimalWindow* window{};

	MemoryArena* arena{};
	MemoryArena perFrameArena{};
	MemoryArena perLayoutArena{};
};

static std::unordered_map<uint32_t, uint32_t> windowLayoutElementsIndices{};
static UIContext* uiContext;

void initRenderContext(MemoryArena* arena);
void cleanupRenderContext();

UIContext* getUIContext();

void onResizeCallback(float width, float height);
void setPointerState(uint32_t windowId, float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown);

void clearPerFrameContext();
void clearPerLayoutContext();

void beginWindow(PrimalWindow* window);
void endWindow();
void beginFrame();
FixedArray<UIWindowBatchCommands> endFrame();

void reset();

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
void pushCircle(float radius, uint32_t segments, float thickness, vec4 color);
void pushCircleFilled(float radius, uint32_t segments, vec4 color);

void pushPanel(UIElementOptions options);

// TitleBar is used by dockspace
void pushTitleBar(UIElementOptions options);
void pushDockSpace(UIElementOptions options);

// Interactions
bool isHovered();
bool isInsideBoundingRect(float x, float y, BoundingRect bb);
void handleDragValue(UILayoutElement* element);

}// namespace pm::UI
