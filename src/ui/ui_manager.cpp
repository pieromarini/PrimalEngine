#include "ui_manager.h"

namespace pm::UI {

UIRenderContext initRenderContext() {
	return {};
}

void begin(UIRenderContext* context) {
	context->textElements.clear();
	context->elements.clear();
	context->indices.clear();
	context->vertices.clear();
	context->fontInfo = {};
}

void end(UIRenderContext* context) {

}

void setFont(UIRenderContext* context, FontInfo fontInfo) {
	context->fontInfo = fontInfo;
}

void pushText(UIRenderContext* context, std::string_view textValue, UIElementOptions options) {
	context->textElements.push_back(text(textValue, context->fontInfo.textureWidth, context->fontInfo.fontChars, options, context->vertices, context->indices));
}

void pushBox(UIRenderContext* context, UIElementOptions options) {
	context->elements.push_back(box(options, context->vertices, context->indices));
}

void pushTriangle(UIRenderContext* context, UIElementOptions options) {
	context->elements.push_back(triangle(options, context->vertices, context->indices));
}

}// namespace pm::UI
