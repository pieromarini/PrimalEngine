#include "ui_widgets.h"
#include "ui/ui_manager.h"

namespace pm {

UI_Signal UI_Label(String8 string) {
	UIElement* element = UIElement_createFromKey(UIElementFlag_DrawText, UIKey{0});
	UIElement_EquipText(element, string);
	UI_Signal result = UI_signalFromElement(element);
	return result;
}

UI_Signal UI_LabelF(char* fmt, ...) {
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
