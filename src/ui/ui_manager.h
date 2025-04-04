#include "utils/fonts.h"
#include "ui_types.h"
#include <stack>
#include <vector>

namespace pm::UI {

enum UIRenderCommandType {
	RECTANGLE,
	TEXT
};

struct UIRenderCommand {
	uint32_t id;
	uint32_t zindex;
	BoundingBox boundingBox;
	glm::vec4 backgroundColor;
	std::string text;
	UIRenderCommandType commandType;
};

struct UIContext {
	std::vector<UIRenderCommand> renderCommands;

	std::vector<UILayoutElement> layoutElements;
	std::vector<uint32_t> layoutElementChildrenIndices;
	std::stack<uint32_t> openLayoutElements; // elements with an open Layout

	std::vector<UIElement> uiElements;
	std::vector<UITextElement> textElements;

	// Fonts
	FontInfo fontInfo;

	float windowWidth, windowHeight;
};

static UIContext* uiContext;

struct InitRenderContextOptions {
	float width, height;
};

void initRenderContext(InitRenderContextOptions options);
void cleanupRenderContext();

UIContext* getUIContext();

// function that should be registered to be called when a window resize occurs
void onResizeCallback(float width, float height);
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

// API
void setFont(FontInfo fontInfo);

void pushText(UIElementOptions options);
void pushBox(UIElementOptions options);
void pushTriangle(UIElementOptions options);

}// namespace pm::UI
