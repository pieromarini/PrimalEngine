#include "ui_widgets.h"
#include "primal_engine.h"
#include "ui/ui_manager.h"

namespace pm {

void UI_Spacer(UI_Size size) {
	auto* parent = UI_topParent();
	UI_setNextPrefSize(parent->childLayoutAxis, size);
	UI_setNextPrefSize(Axis2D_Flip(parent->childLayoutAxis), UI_Pixels(0, 0));
	UIElement_create(0, Str8L(""));
}

UI_Signal UI_Label(String8 string) {
	UIElement* element = UIElement_createFromKey(UIElementFlag_DrawText, UIKey{ 0 });
	UIElement_EquipText(element, string);
	UI_Signal result = UI_signalFromElement(element);
	return result;
}

UI_Signal UI_LabelF(const char* fmt, ...) {
	auto scratch = ScratchBegin();

	va_list args = nullptr;
	va_start(args, fmt);
	String8 string = PushStr8FV(scratch.arena, fmt, args);
	UI_Signal result = UI_Label(string);
	va_end(args);

	ScratchEnd(scratch);
	return result;
}

UI_Signal UI_Button(String8 string) {
	UI_setNextHoverCursor(OS_SYSTEM_CURSOR_POINTER);
	UIElement* element = UIElement_create(UIElementFlag_DrawBorder | UIElementFlag_DrawBackground | UIElementFlag_DrawText | UIElementFlag_DrawHotEffects | UIElementFlag_DrawActiveEffects | UIElementFlag_Clickable, string);
	UI_Signal result = UI_signalFromElement(element);
	return result;
}

UI_Signal UI_Check(b32 checked, String8 string) {
	auto check_size = UI_Em(1.f, 1.f);
	auto padding_size = UI_Em(0.4f, 1.f);
	UI_setNextHoverCursor(OS_SYSTEM_CURSOR_POINTER);
	UI_setNextChildLayoutAxis(Axis2D_X);
	UIElement* element = UIElement_create(UIElementFlag_DrawBorder | UIElementFlag_DrawBackground | UIElementFlag_DrawHotEffects | UIElementFlag_DrawActiveEffects | UIElementFlag_Clickable, string);
	UI_parent(element) UI_padding(padding_size) {
		UI_prefWidth(UI_SizeByChildren(1)) UI_column UI_padding(UI_Pct(1, 0)) {
			UI_setNextPrefWidth(check_size);
			UI_setNextPrefHeight(check_size);
			UI_ElementFlags check_area_flags = UIElementFlag_DrawBackground | UIElementFlag_DisableTextTruncate;
			if (checked) {
				check_area_flags |= UIElementFlag_DrawText;
				UI_setNextTextColor(UI_topFillColor());
				UI_setNextBackgroundColor(mix(UI_topFillColor(), UI_topBackgroundColor(), 0.6f));
				UI_setNextTextAlignment(UITextAlignment_Center);
			}
			UI_setNextFont(&PrimalEngine::get().rendererContext.iconFont);
			UI_setNextFontSize(70.0f);
			UIElement_create(check_area_flags, "%c", 103);
		}
		UI_prefWidth(UI_Pct(1, 0)) UI_Label(string);
	}

	UI_Signal result = UI_signalFromElement(element);

	return result;
}

UI_Signal UI_CheckF(b32 checked, char* fmt, ...) {
	Temp scratch = ScratchBegin();

	va_list args;
	va_start(args, fmt);
	String8 string = PushStr8FV(scratch.arena, fmt, args);
	UI_Signal result = UI_Check(checked, string);
	va_end(args);

	ScratchEnd(scratch);

	return result;
}

void UI_namedColumnBegin(String8 string) {
	UI_setNextChildLayoutAxis(Axis2D_Y);
	UIElement* element = UIElement_create(0, string);
	UI_pushParent(element);
}

void UI_namedColumnBeginF(char* fmt, ...) {
	Temp scratch = ScratchBegin();

	va_list args;
	va_start(args, fmt);
	String8 string = PushStr8FV(scratch.arena, fmt, args);
	UI_namedColumnBegin(string);
	va_end(args);

	ScratchEnd(scratch);
}

void UI_columnBegin() {
	UI_namedColumnBegin(Str8L(""));
}

void UI_columnEnd() {
	UI_popParent();
}

void UI_namedRowBegin(String8 string) {
	UI_setNextChildLayoutAxis(Axis2D_X);
	UIElement* element = UIElement_create(0, string);
	UI_pushParent(element);
}

void UI_namedRowBeginF(char* fmt, ...) {
	Temp scratch = ScratchBegin();

	va_list args;
	va_start(args, fmt);
	String8 string = PushStr8FV(scratch.arena, fmt, args);
	UI_namedRowBegin(string);
	va_end(args);

	ScratchEnd(scratch);
}

void UI_rowBegin() {
	UI_namedRowBegin(Str8L(""));
}

void UI_rowEnd() {
	UI_popParent();
}

};// namespace pm
