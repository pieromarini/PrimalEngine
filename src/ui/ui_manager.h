#include "utils/fonts.h"
#include "ui_types.h"
#include <stack>
#include <vector>

namespace pm::UI {

struct UIContext {
	std::vector<UIRenderCommand> renderCommands;

	std::vector<UILayoutElement> layoutElements;
	std::vector<UILayoutElementData> layoutElementsData;
	std::vector<uint32_t> layoutElementChildrenIndices;
	std::stack<uint32_t> openLayoutElements; // elements with an open Layout


	// Interactions
	PointerState pointerState;
	InteractionState interactionState;
	std::vector<uint32_t> hoveredIds;

	// Fonts
	FontInfo fontInfo;

	// Window context
	float windowWidth, windowHeight;
};

static UIContext* uiContext;

struct InitRenderContextOptions {
	float width, height;
};

void initRenderContext(InitRenderContextOptions options);
void cleanupRenderContext();

UIContext* getUIContext();

void onResizeCallback(float width, float height);
void setPointerState(float mouseX, float mouseY, float relMouseX, float relMouseY, bool isPointerDown);

BoundingBox getTextSize();

void clearContext();

void beginLayout();
std::vector<UIRenderCommand> endLayout();

void openElement();
void closeElement();

void openTextElement();
void closeTextElement();

// Utils
void getTextDimensions(std::string_view text, FontInfo& fontInfo, float& width, float& height);

// Layout
void computeFinalSizes();
void calculateFinalLayout();

// Base layout API
void setFont(FontInfo fontInfo);

void pushText(UIElementOptions options);
void pushBox(UIElementOptions options);
void pushTriangle(UIElementOptions options);

// Interactions
bool isHovered();
bool isInsideBoundingBox(float x, float y, BoundingBox bb);

}// namespace pm::UI
