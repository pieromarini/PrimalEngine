#include "primitives.h"
#include "ui_types.h"
#include <vector>

namespace pm::UI {

struct UIRenderContext {
	std::vector<UIElement> elements;
	std::vector<UITextElement> textElements;

	// Fonts
	FontInfo fontInfo;

	// Geometry
	std::vector<UIVertex> vertices{};
	std::vector<uint32_t> indices{};
};

UIRenderContext initRenderContext();

// Clears the passed in context
void begin(UIRenderContext* context);
void end(UIRenderContext* context);

void setFont(UIRenderContext* context, FontInfo fontInfo);

void pushText(UIRenderContext* context, std::string_view textValue, UIElementOptions options);
void pushBox(UIRenderContext* context, UIElementOptions options);
void pushTriangle(UIRenderContext* context, UIElementOptions options);

}// namespace pm::UI
