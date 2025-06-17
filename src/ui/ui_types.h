#pragma once

#include "SDL3/SDL_events.h"
#include "assets/asset.h"
#include "core/core.h"
#include "core/primal_string.h"
#include "platform/os/events.h"
#include "platform/window.h"

namespace pm {

struct UIKey {
	u64 v[1];
};

using UI_ElementFlags = u32;
enum UIElementFlagsBits {
	// NOTE(piero): interaction
	UIElementFlag_Disabled = (1 << 0),
	UIElementFlag_MouseClickable = (1 << 1),
	UIElementFlag_KeyboardClickable = (1 << 2),
	UIElementFlag_FocusHot = (1 << 3),
	UIElementFlag_FocusActive = (1 << 4),
	UIElementFlag_FocusHotDisabled = (1 << 5),
	UIElementFlag_FocusActiveDisabled = (1 << 6),
	UIElementFlag_ViewScroll = (1 << 7),

	// NOTE(piero): layout
	UIElementFlag_FloatingX = (1 << 8),
	UIElementFlag_FloatingY = (1 << 9),
	UIElementFlag_OverflowX = (1 << 10),
	UIElementFlag_OverflowY = (1 << 11),

	// NOTE(piero): appearance
	UIElementFlag_Clip = (1 << 12),
	UIElementFlag_DisableTextTruncate = (1 << 13),
	UIElementFlag_DisableStringHashPart = (1 << 14),
	UIElementFlag_DrawDropShadow = (1 << 15),
	UIElementFlag_DrawText = (1 << 16),
	UIElementFlag_DrawBorder = (1 << 17),
	UIElementFlag_DrawOverlay = (1 << 18),
	UIElementFlag_DrawBackground = (1 << 19),
	UIElementFlag_DrawHotEffects = (1 << 20),
	UIElementFlag_DrawActiveEffects = (1 << 21),
	UIElementFlag_DrawBucket = (1 << 22),
	UIElementFlag_DrawCustomFunction = (1 << 23),

	// NOTE(piero): bundles
	UIElementFlag_Floating = UIElementFlag_FloatingX | UIElementFlag_FloatingY,
	UIElementFlag_Clickable = UIElementFlag_MouseClickable | UIElementFlag_KeyboardClickable,
};

enum UI_SizeType {
	UISizeType_Pixels,
	UISizeType_TextDim,
	UISizeType_Pct,
	UISizeType_SizeByChildren,
	UISizeType_COUNT
};

enum Corner {
	Corner_Invalid = -1,
	Corner_00,
	Corner_01,
	Corner_10,
	Corner_11,
	Corner_COUNT
};

struct UI_Size {
	UI_SizeType type;
	f32 value;
	f32 strictness;
};

#define UI_Pixels(v, s) \
	UI_Size { .type = UISizeType_Pixels, .value = v, .strictness = s }
#define UI_TextDim(s) \
	UI_Size { .type = UISizeType_TextDim, .value = 0, .strictness = s }
#define UI_SizeByChildren(s) \
	UI_Size { .type = UISizeType_SizeByChildren, .value = 0, .strictness = s }
#define UI_Pct(v, s) \
	UI_Size { .type = UISizeType_Pct, .value = v, .strictness = s }
#define UI_Em(v, s) \
	UI_Size { .type = UISizeType_Pixels, .value = v, .strictness = s }

enum UI_FocusKind : u32 {
	UIFocusKind_Null,
	UIFocusKind_On,
	UIFocusKind_Off,
	UIFocusKind_Root,
	UIFocusKind_COUNT
};

enum UI_MouseButtonSlot : u32 {
	UIMouseButtonSlot_Left,
	UIMouseButtonSlot_Middle,
	UIMouseButtonSlot_Right,
	UIMouseButtonSlot_COUNT
};

enum UI_TextAlignment : u32 {
	UITextAlignment_Left,
	UITextAlignment_Center,
	UITextAlignment_Right,
	UITextAlignment_COUNT,
};

struct UIElement_TextExt {
	UI_TextAlignment textAlignment;
	f32 textEdgePadding;
	f32 fontSize;
	FontAsset* font;
	vec4 textColor;
	String8 string;
};

struct UIElement_RectStyleExt {
	f32 cornerRadii[Corner_COUNT]{ 0.0f, 0.0f, 0.0f, 0.0f };
	f32 borderThickness{ 0.0f };
	f32 softness{ 0.0f };
	vec4 backgroundColor{ 0.0f };
	vec4 borderColor{ 0.0f };
	vec4 overlayColor{ 0.0f };
};

struct UIElement {
	// hash links (cross-frame)
	UIElement* hashNext;
	UIElement* hashPrev;

	// tree links (per-frame)
	UIElement* first;
	UIElement* last;
	UIElement* next;
	UIElement* prev;
	UIElement* parent;
	u64 childCount;

	UIKey key;

	// per-frame params
	UI_ElementFlags flags;
	UI_Size prefSize[Axis2D_COUNT];
	Axis2D childLayoutAxis;
	// TODO(piero): Check how to set cursor in SDL3
	u64 hoverCursor;
	f32 opacity;
	f32 padding;

	UIElement_TextExt* textEXT;
	UIElement_RectStyleExt* rectStyleEXT;

	// post size-calculation data
	vec2 calcSize;
	vec2 calcRelPos;

	// post-layout data
	Rect2D relRect;
	vec2 relCornerDelta[Corner_COUNT];
	Rect2D rect;

	// Cross-frame state
	f32 hotT;
	f32 activeT;
	f32 disabledT;
	f32 focusHotT;
	f32 focusActiveT;
	u64 firstGenTouched;
	u64 lastGenTouched;

	vec2 viewOff;
	vec2 targetViewOff;
};

struct UIElement_Rec {
	UIElement* next;
	i32 pushCount;
	i32 popCount;
};

using UI_SignalFlags = u32;
enum {
	UISignalFlag_PressedLeft = (1 << 0),
	UISignalFlag_PressedMiddle = (1 << 1),
	UISignalFlag_PressedRight = (1 << 2),
	UISignalFlag_ReleasedLeft = (1 << 3),
	UISignalFlag_ReleasedMiddle = (1 << 4),
	UISignalFlag_ReleasedRight = (1 << 5),
	UISignalFlag_ClickedLeft = (1 << 6),
	UISignalFlag_ClickedMiddle = (1 << 7),
	UISignalFlag_ClickedRight = (1 << 8),
	UISignalFlag_DraggingLeft = (1 << 9),
	UISignalFlag_DraggingMiddle = (1 << 10),
	UISignalFlag_DraggingRight = (1 << 11),
	UISignalFlag_DoubleClickedLeft = (1 << 12),
	UISignalFlag_DoubleClickedMiddle = (1 << 13),
	UISignalFlag_DoubleClickedRight = (1 << 14),
	UISignalFlag_PressedKeyboard = (1 << 15),
	UISignalFlag_Hovering = (1 << 16),
	UISignalFlag_MouseIsOver = (1 << 17)
};

struct UI_Signal {
	UIElement* element;
	OS_Modifiers modifiers;
	union {
		UI_SignalFlags flags;
		struct {
			b32 pressed_left : 1;
			b32 pressed_middle : 1;
			b32 pressed_right : 1;
			b32 released_left : 1;
			b32 released_middle : 1;
			b32 released_right : 1;
			b32 clicked_left : 1;
			b32 clicked_middle : 1;
			b32 clicked_right : 1;
			b32 dragging_left : 1;
			b32 dragging_middle : 1;
			b32 dragging_right : 1;
			b32 double_clicked_left : 1;
			b32 double_clicked_middle : 1;
			b32 double_clicked_right : 1;
			b32 pressed_keyboard : 1;
			b32 hovering : 1;
			b32 mouse_is_over : 1;
		};
	};
};

// Events
enum UI_CtrlSlot {
	UICtrlSlot_Null,
	UICtrlSlot_Cancel,// "escape" by default
	UICtrlSlot_Accept,// "enter" by default
	UICtrlSlot_Edit,
	UICtrlSlot_Nav,// arrowkeys, home/end, etc. by default
	UICtrlSlot_COUNT
};

enum UI_EventKind {
	UIEventKind_Null,
	UIEventKind_Press,
	UIEventKind_Release,
	UIEventKind_Text,
	UIEventKind_Scroll,
	UIEventKind_COUNT
};

struct UI_Event {
	UI_EventKind kind;
	UI_CtrlSlot ctrl_slot;
	OS_Key key;
	OS_Modifiers modifiers;
	vec2 position;
	vec2 delta;
	String8 string;
};

struct UI_EventNode {
	UI_EventNode* next;
	UI_EventNode* prev;
	UI_Event v;
};

struct UI_EventList {
	UI_EventNode* first;
	UI_EventNode* last;
	u64 count;
};

// TEMP
inline UI_EventKind sdlEventTypeToUIEventKind(SDL_EventType type) {
	switch (type) {
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		return UIEventKind_Press;
	case SDL_EVENT_MOUSE_BUTTON_UP:
		return UIEventKind_Release;
	case SDL_EVENT_MOUSE_WHEEL:
		return UIEventKind_Scroll;

	default:
		return UIEventKind_Null;
	}
}

}// namespace pm
