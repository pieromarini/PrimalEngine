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

	FixedArray<uint32_t> registeredImageIds;
	FixedArray<VkImageView> imageViews;

	// Interactions
	PointerState pointerState;
	InteractionState interactionState;
	FixedArray<uint32_t> hoveredIds;

	// Fonts
	FontAsset* fontAsset;

	// Window context
	float windowWidth, windowHeight;

	MemoryArena* arena;
	MemoryArena tempArena;
};

static UIContext* uiContext;

struct InitRenderContextOptions {
	float width, height;
};

void initRenderContext(MemoryArena* arena, InitRenderContextOptions options);
void cleanupRenderContext();

UIContext* getUIContext();

void onResizeCallback(float width, float height);
void setPointerState(float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown);

void clearContext();

void beginLayout();
FixedArray<UIRenderCommand> endLayout();

void openElement();
void closeElement();

void openTextElement();
void closeTextElement();
void closeCircleElement();
void closeViewportElement();
void closeDockSpaceElement();

inline UILayoutElement* getElementFromId(uint32_t elementId) {
	auto context = getUIContext();
	return FixedArray_get(context->layoutElements, elementId);
}

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

// Textures
// TODO(piero): This technically makes this UI system dependant on Vulkan. We should find a way to make this generic to any renderer.
uint32_t registerImage(AllocatedImage* image);

}// namespace pm::UI
