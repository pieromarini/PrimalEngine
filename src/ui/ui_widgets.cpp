#include "ui_widgets.h"
#include "ui/ui_manager.h"

namespace pm {

void UI_Spacer(UI_Size size) {
	auto* parent = UI_topParent();
	UI_setNextPrefSize(parent->childLayoutAxis, size);
	UI_setNextPrefSize(Axis2D_Flip(parent->childLayoutAxis), UI_Pixels(0, 0));
	UIElement_create(0, Str8L(""));
}

UI_Signal UI_Label(String8 string) {
	UIElement* element = UIElement_createFromKey(UIElementFlag_DrawText, UIKey{0});
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
	// UI_SetNextHoverCursor(OSCursorKind_Hand);
	UIElement* element = UIElement_create(UIElementFlag_DrawBorder | UIElementFlag_DrawBackground | UIElementFlag_DrawText | UIElementFlag_DrawHotEffects | UIElementFlag_DrawActiveEffects | UIElementFlag_Clickable, string);
	UI_Signal result = UI_signalFromElement(element);
	return result;
}

};// namespace pm
