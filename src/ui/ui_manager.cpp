#include <cinttypes>
#include <cmath>

#include "assets/asset.h"
#include "core/core.h"
#include "core/math/math.h"
#include "core/memory/arena.h"
#include "core/primal_string.h"
#include "platform/os/os.h"
#include "ui/generated.h"
#include "ui/ui_types.h"
#include "ui/ui_utils.h"
#include "utils/fonts.h"
#include "ui_manager.h"


namespace pm {

// Nil values
static UIElement nilUIElement = { .first = &nilUIElement, .last = &nilUIElement, .next = &nilUIElement, .prev = &nilUIElement, .parent = &nilUIElement };
static UIElement_TextExt nilUIElementTextExt = {};
static UIElement_RectStyleExt nilUIElementRectStyleExt = {};
static UI_Size nilPrefWidth = { .type = UISizeType_Pixels, .value = 200.0f, .strictness = 1.0f };
static UI_Size nilPrefHeight = { .type = UISizeType_Pixels, .value = 2.0f, .strictness = 1.0f };
static FontAsset nilFontAsset = {};

__declspec(thread) UIContext* uiContext = nullptr;

UIKey UI_keyZero() {
	UIKey key = { 0 };
	return key;
}

void UI_setCurrentContext(UIContext* context) {
	uiContext = context;
}

UIContext* UI_currentContext() {
	return uiContext;
}

UIContext* UI_createContext() {
	Arena* arena = arenaAlloc(Gigabytes(4));
	auto* context = PushStruct(arena, UIContext);
	context->arena = arena;
	context->elementTableSize = 4096;
	context->elementTable = PushArray(arena, UIElementSlot, context->elementTableSize);

	auto scratch = ScratchBegin();
	for (u64 idx = 0; idx < ArrayCount(context->buildArenas); idx++) {
		String8 name = PushStr8F(scratch.arena, "BuildArena %d", idx);
		context->buildArenas[idx] = arenaAlloc({ .reserveSize = Gigabytes(2), .name = name });
	}
	ScratchEnd(scratch);

	context->dragDataArena = arenaAlloc(Megabytes(64));

	// init nil values
	StackInitNils(context, parent, &nilUIElement);
	StackInitNils(context, prefWidth, nilPrefWidth);
	StackInitNils(context, prefHeight, nilPrefHeight);
	StackInitNils(context, childLayoutAxis, Axis2D_X);
	StackInitNils(context, fixedX, 0.0f);
	StackInitNils(context, fixedY, 0.0f);
	StackInitNils(context, seedKey, UI_keyZero());
	StackInitNils(context, flags, 0);

	StackInitNils(context, focusHot, UIFocusKind_Null);
	StackInitNils(context, focusActive, UIFocusKind_Null);

	StackInitNils(context, hoverCursor, OS_SYSTEM_CURSOR_DEFAULT);
	StackInitNils(context, opacity, 0.0f);

	// Text decorations
	StackInitNils(context, textAlignment, UITextAlignment_Left);
	StackInitNils(context, textEdgePadding, 1.0f);
	StackInitNils(context, font, &nilFontAsset);
	StackInitNils(context, fontSize, 12.0f);
	StackInitNils(context, textColor, (vec4{ 1.0f, 1.0f, 1.0f, 1.0f }));

	// Rect decorations
	StackInitNils(context, cornerRadius00, 0.0f);
	StackInitNils(context, cornerRadius01, 0.0f);
	StackInitNils(context, cornerRadius10, 0.0f);
	StackInitNils(context, cornerRadius11, 0.0f);
	StackInitNils(context, borderThickness, 1.0f);
	StackInitNils(context, backgroundColor, (vec4{ 0.3f, 0.3f, 0.3f, 1.0f }));
	StackInitNils(context, borderColor, (vec4{ 0.3f, 0.3f, 0.3f, 1.0f }));
	StackInitNils(context, overlayColor, (vec4{ 0.3f, 0.3f, 0.3f, 1.0f }));

	StackInitNils(context, fillColor, (vec4{ 0.4f, 0.95f, 1.f, 0.3f }));

	return context;
}

void UI_destroyContext(UIContext* context) {
	arenaRelease(context->dragDataArena);
	for (auto& buildArena : context->buildArenas) {
		arenaRelease(buildArena);
	}
	arenaRelease(context->arena);
}

Arena* getBuildArena() {
	return uiContext->buildArenas[uiContext->buildGen % ArrayCount(uiContext->buildArenas)];
}

UIElement_Rec UIElement_recurseDepthFirst(UIElement* element, UIElement* stopper, MemberOffset sib, MemberOffset child) {
	UIElement_Rec rec = { .next = nullptr };
	rec.next = &nilUIElement;

	if (!UIElement_isNil(MemberFromOff(element, UIElement*, child))) {
		rec.next = MemberFromOff(element, UIElement*, child);
		rec.pushCount = 1;
	} else
		for (UIElement* e = element; !UIElement_isNil(e) && e != stopper; e = e->parent) {
			if (!UIElement_isNil(MemberFromOff(e, UIElement*, sib))) {
				rec.next = MemberFromOff(e, UIElement*, sib);
				break;
			}
			rec.popCount += 1;
		}
	return rec;
}

// Start UI Element

bool UIElement_isNil(UIElement* element) {
	return element == nullptr || element == &nilUIElement;
}

UIElement* UIElement_create(UI_ElementFlags flags, const char* fmt, ...) {
	auto scratch = ScratchBegin();

	va_list args;
	va_start(args, fmt);
	String8 string = PushStr8FV(scratch.arena, fmt, args);
	UIElement* result = UIElement_create(flags, string);
	va_end(args);

	ScratchEnd(scratch);

	return result;
}

UIElement* UIElement_create(UI_ElementFlags flags, String8 str) {
	UIKey seed = UI_topSeedKey();

	// produce a key from the string
	String8 string_hash_part = UI_HashPartFromKeyString(str);
	UIKey key = UI_KeyFromString(seed, string_hash_part);

	// build the element from the key
	UIElement* element = UIElement_createFromKey(flags, key);

	// equip box with text rendering info
	if (flags & UIElementFlag_DrawText) {
		String8 text = UI_TextPartFromKeyString(str);
		UIElement_EquipText(element, text);
	}

	return element;
}

UIElement* UIElement_createFromKey(UI_ElementFlags flags, UIKey key) {
	auto* element = UIElement_fromKey(key);

	if (element->lastGenTouched == uiContext->buildGen) {
		element = &nilUIElement;
		key = UI_keyZero();
	}

	b32 firstFrame = 0;
	if (UIElement_isNil(element)) {
		u64 slot = key.v[0] % uiContext->elementTableSize;
		firstFrame = 1;
		element = uiContext->firstFreeElement;
		if (UIElement_isNil(element)) {
			element = PushStruct(uiContext->arena, UIElement);
		} else {
			StackPop(uiContext->firstFreeElement);
			MemoryZeroStruct(element);
			uiContext->freeElementListCount -= 1;
		}
		DLLPushBack_NPZ(uiContext->elementTable[slot].first, uiContext->elementTable[slot].last, element, hashNext, hashPrev, UIElement_isNil, UIElement_setNil);
		element->key = key;
	}

	auto parent = UI_topParent();
	if (UIElement_isNil(parent)) {
		uiContext->root = element;
	} else {
		DLLPushBack_NPZ(parent->first, parent->last, element, next, prev, UIElement_isNil, UIElement_setNil);
		parent->childCount++;
		element->parent = parent;
	}

	if (!UIElement_isNil(element)) {
		element->childCount = 0;
		element->first = element->last = &nilUIElement;
		element->flags = flags | UI_topFlags();
		element->flags |= UIElementFlag_FocusHot * !!UI_isFocusHot();
		element->flags |= UIElementFlag_FocusHotDisabled * (!UI_isFocusHot() && uiContext->focusHotStack.top->value == UIFocusKind_On);
		element->flags |= UIElementFlag_FocusActive * !!UI_isFocusActive();
		element->flags |= UIElementFlag_FocusActiveDisabled * (!UI_isFocusActive() && uiContext->focusActiveStack.top->value == UIFocusKind_On);
		element->prefSize[Axis2D_X] = UI_topPrefWidth();
		element->prefSize[Axis2D_Y] = UI_topPrefHeight();
		element->childLayoutAxis = UI_topChildLayoutAxis();

		element->hoverCursor = UI_topHoverCursor();
		element->opacity = UI_topOpacity();

		element->lastGenTouched = uiContext->buildGen;

		element->textEXT = &nilUIElementTextExt;
		element->rectStyleEXT = &nilUIElementRectStyleExt;

		if (element->flags & UIElementFlag_DrawText) {
			element->textEXT = PushStruct(getBuildArena(), UIElement_TextExt);
			element->textEXT->font = UI_topFont();
			element->textEXT->fontSize = UI_topFontSize();
			element->textEXT->textAlignment = UI_topTextAlignment();
			element->textEXT->textEdgePadding = UI_topTextEdgePadding();
			element->textEXT->textColor = UI_topTextColor();
		}

		if (element->flags & (UIElementFlag_DrawBackground | UIElementFlag_DrawBorder | UIElementFlag_DrawOverlay)) {
			element->rectStyleEXT = PushStruct(getBuildArena(), UIElement_RectStyleExt);
			element->rectStyleEXT->backgroundColor = UI_topBackgroundColor();
			element->rectStyleEXT->borderColor = UI_topBorderColor();
			element->rectStyleEXT->overlayColor = UI_topOverlayColor();
			element->rectStyleEXT->cornerRadii[Corner_00] = UI_topCornerRadius00();
			element->rectStyleEXT->cornerRadii[Corner_01] = UI_topCornerRadius01();
			element->rectStyleEXT->cornerRadii[Corner_10] = UI_topCornerRadius10();
			element->rectStyleEXT->cornerRadii[Corner_11] = UI_topCornerRadius11();
			element->rectStyleEXT->borderThickness = UI_topBorderThickness();
		}

		// fill fixed positions
		element->calcRelPos.x = UI_topFixedX();
		element->calcRelPos.y = UI_topFixedY();

		// fill first-frame context
		if (firstFrame) {
			element->firstGenTouched = uiContext->buildGen;
		}

		// is focused -> disable per stack
		if (element->flags & UIElementFlag_FocusHot && !UI_isFocusHot()) {
			element->flags |= UIElementFlag_FocusHotDisabled;
		}
		if (element->flags & UIElementFlag_FocusActive && !UI_isFocusActive()) {
			element->flags |= UIElementFlag_FocusActiveDisabled;
		}
	}

	UI_autoPopStacks(uiContext);

	return element;
}

// Maps a key to an Element
UIElement* UIElement_fromKey(UIKey key) {
	UIElement* result = &nilUIElement;
	u64 slot = key.v[0] % uiContext->elementTableSize;
	if (!UI_KeyMatch(key, UI_keyZero())) {
		for (UIElement* b = uiContext->elementTable[slot].first; !UIElement_isNil(b); b = b->hashNext) {
			if (UI_KeyMatch(b->key, key)) {
				result = b;
				break;
			}
		}
	}
	return result;
}

// End UI Element


// Start events

OS_Key UI_OSKeyFromMouseButtonSlot(UI_MouseButtonSlot slot) {
	OS_Key key = OS_Key_Null;
	switch (slot) {
	default: {
	} break;
	case UIMouseButtonSlot_Left: {
		key = OS_Key_MouseLeft;
	} break;
	case UIMouseButtonSlot_Middle: {
		key = OS_Key_MouseMiddle;
	} break;
	case UIMouseButtonSlot_Right: {
		key = OS_Key_MouseRight;
	} break;
	}
	return key;
}

UI_MouseButtonSlot UI_mouseButtonSlotFromOSKey(OS_Key key) {
	UI_MouseButtonSlot slot = UIMouseButtonSlot_Left;
	switch (key) {
	default: {
	} break;
	case OS_Key_MouseLeft: {
		slot = UIMouseButtonSlot_Left;
	} break;
	case OS_Key_MouseMiddle: {
		slot = UIMouseButtonSlot_Middle;
	} break;
	case OS_Key_MouseRight: {
		slot = UIMouseButtonSlot_Right;
	} break;
	}
	return slot;
}

void UI_eatEvent(UI_EventList* events, UI_Event* event) {
	auto* node = BaseFromMember(UI_EventNode, v, event);
	DLLRemove(events->first, events->last, node);
	events->count -= 1;
}

b32 UI_keyPress(UI_EventList* events, OS_Key key, OS_Modifiers mods) {
	b32 result = 0;
	for (UI_EventNode* n = events->first; n != nullptr; n = n->next) {
		if (n->v.kind == UIEventKind_Press && n->v.key == key && n->v.modifiers == mods) {
			UI_eatEvent(events, &n->v);
			result = 1;
			break;
		}
	}
	return result;
}

b32 UI_keyRelease(UI_EventList* events, OS_Key key, OS_Modifiers mods) {
	b32 result = 0;
	for (UI_EventNode* n = events->first; n != nullptr; n = n->next) {
		if (n->v.kind == UIEventKind_Release && n->v.key == key && n->v.modifiers == mods) {
			UI_eatEvent(events, &n->v);
			result = 1;
			break;
		}
	}
	return result;
}

b32 UI_ctrlPress(UI_EventList* events, UI_CtrlSlot slot) {
	b32 result = 0;
	for (UI_EventNode* n = events->first; n != nullptr; n = n->next) {
		if (n->v.kind == UIEventKind_Press && n->v.ctrl_slot == slot) {
			UI_eatEvent(events, &n->v);
			result = 1;
			break;
		}
	}
	return result;
}

b32 UI_ctrlRelease(UI_EventList* events, UI_CtrlSlot slot) {
	b32 result = 0;
	for (UI_EventNode* n = events->first; n != nullptr; n = n->next) {
		if (n->v.kind == UIEventKind_Release && n->v.ctrl_slot == slot) {
			UI_eatEvent(events, &n->v);
			result = 1;
			break;
		}
	}
	return result;
}

// End Events

// Start Interactions

b32 UI_isFocusHot() {
	b32 result = 0;
	for (FocusHotNode* n = uiContext->focusHotStack.top; n != nullptr; n = n->next) {
		switch (n->value) {
		default: {
		} break;
		case UIFocusKind_On: {
			result = 1;
		} break;
		case UIFocusKind_Off: {
			result = 0;
		}
			goto break_all;
		case UIFocusKind_Root: {
		}
			goto break_all;
		}
	}
break_all:;
	return result;
}

b32 UI_isFocusActive() {
	b32 result = 0;
	for (FocusActiveNode* n = uiContext->focusActiveStack.top; n != nullptr; n = n->next) {
		switch (n->value) {
		default: {
		} break;
		case UIFocusKind_On: {
			result = 1;
		} break;
		case UIFocusKind_Off: {
			result = 0;
		}
			goto break_all;
		case UIFocusKind_Root: {
		}
			goto break_all;
		}
	}
break_all:;
	return result;
}

Rect1DF32 UI_scrollBoundsFromElement(UIElement *element, Axis2D axis) {
	Rect1DF32 bounds = {0, 0};
	for(UIElement *child = element->first; !UIElement_isNil(child); child = child->next) {
		bounds.min = Min(bounds.min, child->calcRelPos[axis]);
		bounds.max = Max(bounds.max, child->calcRelPos[axis]);
	}
	return bounds;
}

UI_Signal UI_signalFromElement(UIElement* element) {
	UI_Signal sig = { .element = element };
	UI_EventList* events = uiContext->events;

	Rect2D clippedRect = element->rect;
	for (UIElement* e = element->parent; !UIElement_isNil(e); e = e->parent) {
		if (e->flags & UIElementFlag_Clip) {
			clippedRect = rect2DIntersect(clippedRect, e->rect);
		}
	}

	for (UI_EventNode *n = events->first, *next = nullptr; n != nullptr; n = next) {
		next = n->next;
		b32 taken = 0;
		UI_Event* ev = &n->v;
		b32 eventInElementInteractionRegion = rect2DContains(clippedRect, ev->position);
		b32 eventKeyIsMouse = (ev->key == OS_Key_MouseLeft || ev->key == OS_Key_MouseRight || ev->key == OS_Key_MouseMiddle);
		UI_MouseButtonSlot ev_mb_slot = UI_mouseButtonSlotFromOSKey(ev->key);

		if (element->firstGenTouched != element->lastGenTouched && element->flags & UIElementFlag_MouseClickable) {
			if (eventKeyIsMouse && eventInElementInteractionRegion && ev->kind == UIEventKind_Press) {
				taken = 1;
				uiContext->hotKey = uiContext->activeKey[ev_mb_slot] = element->key;
				sig.flags |= UISignalFlag_PressedLeft << ev_mb_slot;
				uiContext->dragStartMouse = ev->position;
			}
			if (eventKeyIsMouse && ev->kind == UIEventKind_Release && UI_KeyMatch(uiContext->activeKey[ev_mb_slot], element->key)) {
				taken = 1;
				sig.flags |= UISignalFlag_ReleasedLeft << ev_mb_slot;
				if (eventInElementInteractionRegion) {
					sig.flags |= UISignalFlag_ClickedLeft << ev_mb_slot;
				}
				uiContext->activeKey[ev_mb_slot] = UI_keyZero();
			}
		}

		if (element->flags & UIElementFlag_KeyboardClickable && element->flags & UIElementFlag_FocusHot && !(element->flags & UIElementFlag_FocusHotDisabled) && ev->kind == UIEventKind_Press && ev->ctrl_slot == UICtrlSlot_Accept) {
			taken = 1;
			sig.flags |= UISignalFlag_ClickedLeft | UISignalFlag_PressedLeft | UISignalFlag_PressedKeyboard;
		}

		if(element->flags & UIElementFlag_ViewScroll && ev->kind == UIEventKind_Scroll && eventInElementInteractionRegion) {
			taken = 1;
			for (auto axis = (Axis2D)0; axis < Axis2D_COUNT; axis = Axis2D(axis + 1)) {
				element->targetViewOff[axis] += ev->delta[axis];
				if(element->flags & (UIElementFlag_OverflowX << axis)) {
					UI_layoutRoot(element, axis);
				}
				auto scroll_bounds = UI_scrollBoundsFromElement(element, axis);
				element->targetViewOff[axis] = clamp1F32(scroll_bounds, element->targetViewOff[axis]);
			}
		}

		if (taken) {
			UI_eatEvent(events, ev);
		}
	}

	// fill out flags & state based on polled information
	vec2 mousePosition = uiContext->mouse;
	if (rect2DContains(clippedRect, mousePosition)) {
		sig.flags |= UISignalFlag_MouseIsOver;
	}
	if (rect2DContains(clippedRect, mousePosition)) {
		if (UI_KeyMatch(UI_keyZero(), uiContext->hotKey)) {
			sig.flags |= UISignalFlag_Hovering;
			b32 isAnyKeyActive = 0;
			for (auto slot = (UI_MouseButtonSlot)0; slot < UIMouseButtonSlot_COUNT; slot = UI_MouseButtonSlot(slot + 1)) {
				if (!UI_KeyMatch(uiContext->activeKey[slot], UI_keyZero())) {
					isAnyKeyActive = 1;
					break;
				}
			}
			if (!isAnyKeyActive) {
				uiContext->hotKey = element->key;
			}
		}
	} else if (UI_KeyMatch(element->key, uiContext->hotKey)) {
		uiContext->hotKey = UI_keyZero();
	}
	if (element->flags & UIElementFlag_MouseClickable) {
		for (auto slot = (UI_MouseButtonSlot)0; slot < UIMouseButtonSlot_COUNT; slot = UI_MouseButtonSlot(slot + 1)) {
			if (UI_KeyMatch(uiContext->activeKey[slot], element->key)) {
				sig.flags |= UISignalFlag_DraggingLeft << slot;
				uiContext->hotKey = element->key;
			}
		}
	}

	return sig;
}

// End Interactions


// Start Layout

void UI_layoutRoot(UIElement* root, Axis2D axis) {
	UI_solveIndependentSizes(root, axis);
	UI_solveUpwardDependentSizes(root, axis);
	UI_solveDownwardDependentSizes(root, axis);
	UI_solveSizeViolations(root, axis);
}

void UI_solveIndependentSizes(UIElement* root, Axis2D axis) {
	switch (root->prefSize[axis].type) {
	default:
		break;
	case UISizeType_Pixels: {
		root->calcSize[axis] = root->prefSize[axis].value;
		root->calcSize[axis] = std::floorf(root->calcSize[axis]);
	} break;
	case UISizeType_TextDim: {
		switch (axis) {
		default: {
		} break;

		// TODO(piero): Cache the generated text geometry
		case Axis2D_X: {
			auto position = rect2DSize(root->rect);
			auto textDim = generateTextGeometry(root->textEXT->string, root->textEXT->fontSize, root->textEXT->font, nullptr, nullptr, position);
			root->calcSize[axis] = textDim.x;
			root->calcSize[axis] += root->textEXT->textEdgePadding * 2.0f;
			root->calcSize[axis] = std::ceilf(root->calcSize[axis]);
		} break;

		case Axis2D_Y: {
			MSDFFont fontInfo = root->textEXT->font->metadata;
			auto position = rect2DSize(root->rect);
			auto textDim = generateTextGeometry(root->textEXT->string, root->textEXT->fontSize, root->textEXT->font, nullptr, nullptr, position);
			root->calcSize[axis] = textDim.y;
			root->calcSize[axis] = std::floorf(root->calcSize[axis]);
		} break;
		}
	} break;
	}

	for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
		UI_solveIndependentSizes(child, axis);
	}
}

void UI_solveUpwardDependentSizes(UIElement* root, Axis2D axis) {
	switch (root->prefSize[axis].type) {
	default:
		break;
	case UISizeType_Pct: {
		UIElement* ancestor = &nilUIElement;
		for (UIElement* p = root->parent; !UIElement_isNil(p); p = p->parent) {
			if (p->prefSize[axis].type != UISizeType_SizeByChildren) {
				ancestor = p;
				break;
			}
		}
		if (!UIElement_isNil(ancestor)) {
			root->calcSize[axis] = ancestor->calcSize[axis] * root->prefSize[axis].value;
			root->calcSize[axis] = std::floorf(root->calcSize[axis]);
		}
	} break;
	}
	for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
		UI_solveUpwardDependentSizes(child, axis);
	}
}

void UI_solveDownwardDependentSizes(UIElement* root, Axis2D axis) {
	for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
		UI_solveDownwardDependentSizes(child, axis);
	}
	switch (root->prefSize[axis].type) {
	default:
		break;
	case UISizeType_SizeByChildren: {
		f32 value = 0;
		{
			if (axis == root->childLayoutAxis) {
				for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
					value += child->calcSize[axis];
				}
			} else {
				for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
					value = Max(value, child->calcSize[axis]);
				}
			}
		}
		root->calcSize[axis] = value;
		root->calcSize[axis] = std::floorf(root->calcSize[axis]);
	} break;
	}
}

void UI_solveSizeViolations(UIElement* root, Axis2D axis) {
	// determine the maximum available space
	f32 available_space = root->calcSize[axis];

	// determine the size taken by all of the children's preferred sizes, & the total budget we have to fix the sizes up
	f32 taken_space = 0;
	f32 total_fixup_budget = 0;
	if (!(root->flags & (UIElementFlag_OverflowX << axis))) {
		for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
			if (!(child->flags & (UIElementFlag_FloatingX << axis))) {
				if (axis == root->childLayoutAxis) {
					taken_space += child->calcSize[axis];
				} else {
					taken_space = Max(taken_space, child->calcSize[axis]);
				}
				f32 fixup_budget_this_child = child->calcSize[axis] * (1 - child->prefSize[axis].strictness);
				total_fixup_budget += fixup_budget_this_child;
			}
		}
	}

	// fixup all children as much as possible
	if (!(root->flags & (UIElementFlag_OverflowX << axis))) {
		f32 violation = taken_space - available_space;
		if (violation > 0 && total_fixup_budget > 0) {
			for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
				if (!(child->flags & (UIElementFlag_FloatingX << axis))) {
					f32 fixup_budget_this_child = child->calcSize[axis] * (1 - child->prefSize[axis].strictness);
					f32 fixup_size_this_child = 0;
					if (axis == root->childLayoutAxis) {
						fixup_size_this_child = fixup_budget_this_child * (violation / total_fixup_budget);
					} else {
						fixup_size_this_child = child->calcSize[axis] - available_space;
					}
					fixup_size_this_child = Clamp(0, fixup_size_this_child, fixup_budget_this_child);
					child->calcSize[axis] -= fixup_size_this_child;
					child->calcSize[axis] = std::floorf(child->calcSize[axis]);
				}
			}
		}
	}

	// position all children
	{
		if (axis == root->childLayoutAxis) {
			f32 p = 0;
			for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
				if (!(child->flags & (UIElementFlag_FloatingX << axis))) {
					child->calcRelPos[axis] = p;
					p += child->calcSize[axis];
				}
			}
		} else {
			for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
				if (!(child->flags & (UIElementFlag_FloatingX << axis))) {
					child->calcRelPos[axis] = 0;
				}
			}
		}
		for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
			Rect2D last_relRect = child->relRect;
			child->relRect.min[axis] = child->calcRelPos[axis];
			child->relRect.max[axis] = child->relRect.min[axis] + child->calcSize[axis];
			vec2 last_corner_01 = vec2{ last_relRect.min.x, last_relRect.max.y };
			vec2 last_corner_10 = vec2{ last_relRect.max.x, last_relRect.min.y };
			vec2 this_corner_01 = vec2{ child->relRect.min.x, child->relRect.max.y };
			vec2 this_corner_10 = vec2{ child->relRect.max.x, child->relRect.min.y };
			child->relCornerDelta[Corner_00][axis] = child->relRect.min[axis] - last_relRect.min[axis];
			child->relCornerDelta[Corner_01][axis] = this_corner_01[axis] - last_corner_01[axis];
			child->relCornerDelta[Corner_10][axis] = this_corner_10[axis] - last_corner_10[axis];
			child->relCornerDelta[Corner_11][axis] = child->relRect.max[axis] - last_relRect.max[axis];
			child->rect.min[axis] = root->rect.min[axis] + child->relRect.min[axis] - root->viewOff[axis];
			child->rect.max[axis] = child->rect.min[axis] + child->calcSize[axis];
			if (!(child->flags & (UIElementFlag_FloatingX << axis))) {
				child->rect.min[axis] = std::floorf(child->rect.min[axis]);
				child->rect.max[axis] = std::floorf(child->rect.max[axis]);
			}
		}
	}

	for (UIElement* child = root->first; !UIElement_isNil(child); child = child->next) {
		UI_solveSizeViolations(child, axis);
	}
}
// End Layout

void UIElement_EquipText(UIElement* element, String8 text) {
	if (element->textEXT != &nilUIElementTextExt) {
		element->textEXT->string = PushStr8Copy(getBuildArena(), text);
	}
}

vec2 UI_textPosFromElement(UIElement* element) {
	vec2 result = {};

	auto rectSize = rect2DSize(element->rect);

	auto font = element->textEXT->font;
	f32 fontSize = element->textEXT->fontSize;
	MSDFFont fontMetrics = font->metadata;
	auto textDim = generateTextGeometry(element->textEXT->string, element->textEXT->fontSize, element->textEXT->font, nullptr, nullptr, rectSize);

	result.y = std::floorf((element->rect.min.y + element->rect.max.y) / 2.f) - textDim.y;

	switch (element->textEXT->textAlignment) {
	default:
	case UITextAlignment_Left: {
		result.x = element->rect.min.x + element->textEXT->textEdgePadding;
	} break;
	case UITextAlignment_Center: {
		result.x = std::floorf((element->rect.min.x + element->rect.max.x) / 2 - textDim.x / 2);
		result.x = ClampBot(result.x, element->rect.min.x);
	} break;
	case UITextAlignment_Right: {
		result.x = std::roundf((element->rect.max.x) - textDim.x - element->textEXT->textEdgePadding);
		result.x = ClampBot(result.x, element->rect.min.x);
	} break;
	}
	result.x = std::floorf(result.x);
	return result;
}


void UI_beginBuild(PrimalWindow* window, UI_EventList* events, f32 deltaTime) {
	uiContext->buildGen++;
	arenaClear(getBuildArena());

	uiContext->deltaTime = deltaTime;
	uiContext->root = &nilUIElement;
	uiContext->window = window;
	uiContext->events = events;

	uiContext->mouse = OS_mouseFromWindow();
	MemoryZeroStruct(&uiContext->hotKey);

	// init stacks
	uiContext->parentStack = StackCreate(uiContext, parent);
	uiContext->prefWidthStack = StackCreate(uiContext, prefWidth);
	uiContext->prefHeightStack = StackCreate(uiContext, prefHeight);
	uiContext->childLayoutAxisStack = StackCreate(uiContext, childLayoutAxis);
	uiContext->fixedXStack = StackCreate(uiContext, fixedX);
	uiContext->fixedYStack = StackCreate(uiContext, fixedY);
	uiContext->seedKeyStack = StackCreate(uiContext, seedKey);
	uiContext->flagsStack = StackCreate(uiContext, flags);

	uiContext->focusHotStack = StackCreate(uiContext, focusHot);
	uiContext->focusActiveStack = StackCreate(uiContext, focusActive);

	uiContext->hoverCursorStack = StackCreate(uiContext, hoverCursor);
	uiContext->opacityStack = StackCreate(uiContext, opacity);

	uiContext->textAlignmentStack = StackCreate(uiContext, textAlignment);
	uiContext->textEdgePaddingStack = StackCreate(uiContext, textEdgePadding);
	uiContext->fontStack = StackCreate(uiContext, font);
	uiContext->fontSizeStack = StackCreate(uiContext, fontSize);
	uiContext->textColorStack = StackCreate(uiContext, textColor);

	uiContext->cornerRadius00Stack = StackCreate(uiContext, cornerRadius00);
	uiContext->cornerRadius01Stack = StackCreate(uiContext, cornerRadius01);
	uiContext->cornerRadius10Stack = StackCreate(uiContext, cornerRadius10);
	uiContext->cornerRadius11Stack = StackCreate(uiContext, cornerRadius11);
	uiContext->borderThicknessStack = StackCreate(uiContext, borderThickness);
	uiContext->backgroundColorStack = StackCreate(uiContext, backgroundColor);
	uiContext->borderColorStack = StackCreate(uiContext, borderColor);
	uiContext->overlayColorStack = StackCreate(uiContext, overlayColor);

	uiContext->fillColorStack = StackCreate(uiContext, fillColor);

	// kill action
	if (uiContext->actionKilledThisFrame) {
		uiContext->actionKilledThisFrame = 0;
		MemoryZeroArray(uiContext->activeKey);
	}

	// prune all of the stale boxes
	for (u64 slot = 0; slot < uiContext->elementTableSize; slot += 1) {
		for (UIElement *element = uiContext->elementTable[slot].first, *next = nullptr; !UIElement_isNil(element); element = next) {
			next = element->hashNext;
			if (UI_KeyMatch(element->key, UI_keyZero()) || element->lastGenTouched + 1 < uiContext->buildGen) {
				DLLRemove_NPZ(uiContext->elementTable[slot].first, uiContext->elementTable[slot].last, element, hashNext, hashPrev, UIElement_isNil, UIElement_setNil);
				StackPush(uiContext->firstFreeElement, element);
				uiContext->freeElementListCount += 1;
			}
		}
	}

	// zero hot key on pruned boxes
	UIElement* element = UIElement_fromKey(uiContext->hotKey);
	if (UIElement_isNil(element) && (UI_KeyMatch(UI_keyZero(), uiContext->activeKey[UIMouseButtonSlot_Left]) || !UI_KeyMatch(uiContext->hotKey, uiContext->activeKey[UIMouseButtonSlot_Left])) && (UI_KeyMatch(UI_keyZero(), uiContext->activeKey[UIMouseButtonSlot_Middle]) || !UI_KeyMatch(uiContext->hotKey, uiContext->activeKey[UIMouseButtonSlot_Middle])) && (UI_KeyMatch(UI_keyZero(), uiContext->activeKey[UIMouseButtonSlot_Right]) || !UI_KeyMatch(uiContext->hotKey, uiContext->activeKey[UIMouseButtonSlot_Right]))) {
		uiContext->hotKey = UI_keyZero();
	}

	// create root
	vec2 clientRectSize = { window->width, window->height };
	UI_setNextPrefWidth(UI_Pixels(clientRectSize.x, 1.0f));
	UI_setNextPrefHeight(UI_Pixels(clientRectSize.y, 1.0f));
	UI_setNextChildLayoutAxis(Axis2D_Y);

	UIElement* root = UIElement_create(0, "window_root_%" PRIx64 "", &window->handle);

	UI_pushParent(root);

	// defaults
	// TODO(piero): Set a default font when we have a proper font manager/cache
	UI_pushFontSize(12.f);
	UI_pushBackgroundColor({ 0.1f, 0.13f, 0.14f, 0.7f });
	UI_pushPrefWidth(UI_Pct(1.f, 0.f));
	UI_pushPrefHeight(UI_Em(1.8f, 1.f));
	UI_pushTextColor(vec4{1, 1, 1, 1});
	UI_pushBorderColor(vec4{0, 1, 0, 0.2f});
	UI_pushBorderThickness(1.0f);
	UI_pushTextEdgePadding(UI_topFontSize() * 0.5f);
}

void UI_endBuild() {
	UI_popParent();

	auto hotElement = UIElement_fromKey(uiContext->hotKey);
	OS_setCursor(hotElement->hoverCursor);

	for (auto axis = (Axis2D)0; axis < Axis2D_COUNT; axis = Axis2D(axis + 1)) {
		UI_layoutRoot(uiContext->root, axis);
	}

	for (auto slot = (UI_MouseButtonSlot)0; slot < UIMouseButtonSlot_COUNT; slot = UI_MouseButtonSlot(slot + 1)) {
		UIElement* element = UIElement_fromKey(uiContext->activeKey[slot]);
		if (UIElement_isNil(element)) {
			OS_Key key = UI_OSKeyFromMouseButtonSlot(slot);
			b32 release = 0;
			for (UI_EventNode* n = uiContext->events->first; n != nullptr; n = n->next) {
				UI_Event* event = &n->v;
				if (event->kind == UIEventKind_Release && event->key == key) {
					release = 1;
					break;
				}
			}
			if (release) {
				MemoryZeroStruct(&uiContext->activeKey[slot]);
			}
		}
	}
}

void UI_draw(VulkanRendererContext* context) {
	for (UIElement *element = uiContext->root, *nextBox = &nilUIElement; !UIElement_isNil(element); element = nextBox) {
		auto rec = UIElement_recurseDepthFirstPost(element, &nilUIElement);
		nextBox = rec.next;

		if (element->opacity != 1.0f) {
			Renderer_pushTransparency(context, 1.0f - element->opacity);
		}

		auto dpi = SDL_GetWindowDisplayScale(context->rendererState->window->handle);

		// TODO(piero): Play with these settings/ideas some more... result is not good right now.
		if(element->flags & UIElementFlag_DrawDropShadow) {
			f32 shift = dpi * 0.03f;
			auto shadowRect = rect2DPad(rect2DShift(element->rect, vec2{ shift, shift }), shift * 2.0f);
			UIElement_RectStyleExt style{};
			style.backgroundColor = vec4{ 0.0f, 0.0f, 0.0f, 0.8f };
			style.cornerRadii[0] = style.cornerRadii[1] = style.cornerRadii[2] = style.cornerRadii[3] = dpi * 0.02f;
			// MemoryCopyArray(style.cornerRadii, element->rectStyleEXT->cornerRadii);
			style.softness = 6.0f;
			Renderer_pushRect(context, shadowRect, style);
		}

		if (element->flags & UIElementFlag_DrawBackground) {
			auto rect = element->rect;
			UIElement_RectStyleExt style{};
			style.backgroundColor = element->rectStyleEXT->backgroundColor;
			MemoryCopyArray(style.cornerRadii, element->rectStyleEXT->cornerRadii);
			style.softness = 1.0f;
			Renderer_pushRect(context, rect, style);

			if (element->flags & UIElementFlag_DrawHotEffects) {
				auto activeDestroyer = element->flags & UIElementFlag_DrawActiveEffects ? element->activeT : 0;
				f32 effectiveHotT = element->hotT * (1 - activeDestroyer);
				f32 edgeThickness = dpi * 0.1f;
			}

			if (element->flags & UIElementFlag_DrawActiveEffects) {
			}

			if (element->focusHotT >= 0.005f) {
			}

		}

		if (element->flags & UIElementFlag_DrawText) {
			auto textPos = UI_textPosFromElement(element);
			Renderer_pushText(context, textPos, element->textEXT);
		}

		if (element->flags & UIElementFlag_DrawBorder) {
			auto rect = rect2DPad(element->rect, 1.0f);
			UIElement_RectStyleExt style{};
			MemoryCopyArray(style.cornerRadii, element->rectStyleEXT->cornerRadii);
			// TODO(piero): quick hack for now. The renderer should send 4 color values per rectangle.
			//              We can send 4 color values per vertex, but this seems wasteful and repetitive, since the 4 colors are the same between a rectangle's vertices.
			//              The idea should be to send 4 colors as part of the UIMaterialData. This should be better memory-wise and it will still be useful once we add instancing for rectangle rendering.
			style.backgroundColor = element->rectStyleEXT->borderColor;
			style.borderThickness = element->rectStyleEXT->borderThickness;
			style.softness = 1.0f;
			Renderer_pushRect(context, rect, style);
		}

		if (element->flags & UIElementFlag_Clip) {
		}

		if (rec.pushCount == 0) {
			int pop_idx = 0;
			for (UIElement* p = element; !UIElement_isNil(p) && p != nextBox && pop_idx <= rec.popCount; p = p->parent, pop_idx += 1) {
				if (p->flags & UIElementFlag_Clip) {
				}

				// draw disabled overlay
				if (p->disabledT > 0.01f) {
					auto rect = element->rect;
					UIElement_RectStyleExt style{};
					style.backgroundColor = { 0.0f, 0.0f, 0.0f, 0.6f * p->disabledT };
					style.softness = 1.0f;
					MemoryCopyArray(style.cornerRadii, element->rectStyleEXT->cornerRadii);
					Renderer_pushRect(context, rect, style);
				}

				// pop opacity
				if (p->opacity != 1.0f) {
					Renderer_popTransparency(context);
				}
			}
		}

	}
}

void UI_pushCornerRadius(f32 v) {
 UI_pushCornerRadius00(v);
 UI_pushCornerRadius01(v);
 UI_pushCornerRadius10(v);
 UI_pushCornerRadius11(v);
}

void UI_popCornerRadius() {
 UI_popCornerRadius00();
 UI_popCornerRadius01();
 UI_popCornerRadius10();
 UI_popCornerRadius11();
}

void UI_setNextCornerRadius(f32 v) {
 UI_setNextCornerRadius00(v);
 UI_setNextCornerRadius01(v);
 UI_setNextCornerRadius10(v);
 UI_setNextCornerRadius11(v);
}

void UI_pushPrefSize(Axis2D axis, UI_Size v) {
	(axis == Axis2D_X ? UI_pushPrefWidth : UI_pushPrefHeight)(v);
}

void UI_popPrefSize(Axis2D axis) {
	(axis == Axis2D_X ? UI_popPrefWidth : UI_popPrefHeight)();
}

void UI_setNextPrefSize(Axis2D axis, UI_Size v) {
	(axis == Axis2D_X ? UI_setNextPrefWidth : UI_setNextPrefHeight)(v);
}

void UI_pushFixedPos(vec2 v) {
	UI_pushFixedX(v.x);
	UI_pushFixedY(v.y);
}

void UI_popFixedPos() {
	UI_popFixedX();
	UI_popFixedY();
}

void UI_setNextFixedPos(vec2 v) {
	UI_setNextFixedX(v.x);
	UI_setNextFixedY(v.y);
}

void UI_pushFixedRect(Rect2D rect) {
	vec2 dim = rect2DSize(rect);
	UI_pushFixedPos(rect.min);
	UI_pushPrefSize(Axis2D_X, UI_Pixels(dim.x, 1));
	UI_pushPrefSize(Axis2D_Y, UI_Pixels(dim.y, 1));
}

void UI_popFixedRect() {
	UI_popFixedPos();
	UI_popPrefSize(Axis2D_X);
	UI_popPrefSize(Axis2D_Y);
}

void UI_setNextFixedRect(Rect2D rect) {
	vec2 dim = rect2DSize(rect);
	UI_setNextFixedPos(rect.min);
	UI_setNextPrefSize(Axis2D_X, UI_Pixels(dim.x, 1));
	UI_setNextPrefSize(Axis2D_Y, UI_Pixels(dim.y, 1));
}

void UI_storeDragData(vec2 data) {
	arenaClear(uiContext->dragDataArena);
	uiContext->dragData = PushStruct(uiContext->dragDataArena, vec2);
	uiContext->dragData->x = data.x;
	uiContext->dragData->y = data.y;
}

vec2 UI_loadDragData() {
	return *uiContext->dragData;
}

vec2 UI_dragDelta() {
	return (uiContext->mouse - uiContext->dragStartMouse);
}

// Local macros to use specific arenas for stacks
#define StackPushImpl(state, name_upper, name_lower, new_value) StackPushImplArena(state, name_upper, name_lower, new_value, getBuildArena())
#define StackSetNextImpl(state, name_upper, name_lower, new_value) StackSetNextImplArena(state, name_upper, name_lower, new_value, getBuildArena())

// Generated
UIElement* UI_topParent() { StackTopImpl(uiContext, Parent, parent) }
UIElement* UI_pushParent(UIElement* value) { StackPushImpl(uiContext, Parent, parent, value) }
UIElement* UI_popParent() { StackPopImpl(uiContext, Parent, parent) }
UIElement* UI_setNextParent(UIElement* value) { StackSetNextImpl(uiContext, Parent, parent, value) }

UI_Size UI_topPrefWidth() { StackTopImpl(uiContext, PrefDim, prefWidth) }
UI_Size UI_pushPrefWidth(UI_Size value) { StackPushImpl(uiContext, PrefDim, prefWidth, value) }
UI_Size UI_popPrefWidth() { StackPopImpl(uiContext, PrefDim, prefWidth) }
UI_Size UI_setNextPrefWidth(UI_Size value) { StackSetNextImpl(uiContext, PrefDim, prefWidth, value) }

UI_Size UI_topPrefHeight() { StackTopImpl(uiContext, PrefDim, prefHeight) }
UI_Size UI_pushPrefHeight(UI_Size value) { StackPushImpl(uiContext, PrefDim, prefHeight, value) }
UI_Size UI_popPrefHeight() { StackPopImpl(uiContext, PrefDim, prefHeight) }
UI_Size UI_setNextPrefHeight(UI_Size value) { StackSetNextImpl(uiContext, PrefDim, prefHeight, value) }

Axis2D UI_topChildLayoutAxis() { StackTopImpl(uiContext, ChildLayoutAxis, childLayoutAxis) }
Axis2D UI_pushChildLayoutAxis(Axis2D value) { StackPushImpl(uiContext, ChildLayoutAxis, childLayoutAxis, value) }
Axis2D UI_popChildLayoutAxis() { StackPopImpl(uiContext, ChildLayoutAxis, childLayoutAxis) }
Axis2D UI_setNextChildLayoutAxis(Axis2D value) { StackSetNextImpl(uiContext, ChildLayoutAxis, childLayoutAxis, value) }

f32 UI_topFixedX() { StackTopImpl(uiContext, FixedX, fixedX) }
f32 UI_pushFixedX(f32 value) { StackPushImpl(uiContext, FixedX, fixedX, value) }
f32 UI_popFixedX() { StackPopImpl(uiContext, FixedX, fixedX) }
f32 UI_setNextFixedX(f32 value) { StackSetNextImpl(uiContext, FixedX, fixedX, value) }

f32 UI_topFixedY() { StackTopImpl(uiContext, FixedY, fixedY) }
f32 UI_pushFixedY(f32 value) { StackPushImpl(uiContext, FixedY, fixedY, value) }
f32 UI_popFixedY() { StackPopImpl(uiContext, FixedY, fixedY) }
f32 UI_setNextFixedY(f32 value) { StackSetNextImpl(uiContext, FixedY, fixedY, value) }

UIKey UI_topSeedKey() { StackTopImpl(uiContext, SeedKey, seedKey) }
UIKey UI_pushSeedKey(UIKey value) { StackPushImpl(uiContext, SeedKey, seedKey, value) }
UIKey UI_popSeedKey() { StackPopImpl(uiContext, SeedKey, seedKey) }
UIKey UI_setNextSeedKey(UIKey value) { StackSetNextImpl(uiContext, SeedKey, seedKey, value) }

UI_ElementFlags UI_topFlags() { StackTopImpl(uiContext, Flags, flags) }
UI_ElementFlags UI_pushFlags(UI_ElementFlags value) { StackPushImpl(uiContext, Flags, flags, value) }
UI_ElementFlags UI_popFlags() { StackPopImpl(uiContext, Flags, flags) }
UI_ElementFlags UI_setNextFlags(UI_ElementFlags value) { StackSetNextImpl(uiContext, Flags, flags, value) }

UI_FocusKind UI_topFocusHot() { StackTopImpl(uiContext, FocusHot, focusHot) }
UI_FocusKind UI_pushFocusHot(UI_FocusKind value) { StackPushImpl(uiContext, FocusHot, focusHot, value) }
UI_FocusKind UI_popFocusHot() { StackPopImpl(uiContext, FocusHot, focusHot) }
UI_FocusKind UI_setNextFocusHot(UI_FocusKind value) { StackSetNextImpl(uiContext, FocusHot, focusHot, value) }

UI_FocusKind UI_topFocusActive() { StackTopImpl(uiContext, FocusActive, focusActive) }
UI_FocusKind UI_pushFocusActive(UI_FocusKind value) { StackPushImpl(uiContext, FocusActive, focusActive, value) }
UI_FocusKind UI_popFocusActive() { StackPopImpl(uiContext, FocusActive, focusActive) }
UI_FocusKind UI_setNextFocusActive(UI_FocusKind value) { StackSetNextImpl(uiContext, FocusActive, focusActive, value) }

OS_CursorType UI_topHoverCursor() { StackTopImpl(uiContext, HoverCursor, hoverCursor) }
OS_CursorType UI_pushHoverCursor(OS_CursorType value) { StackPushImpl(uiContext, HoverCursor, hoverCursor, value) }
OS_CursorType UI_popHoverCursor() { StackPopImpl(uiContext, HoverCursor, hoverCursor) }
OS_CursorType UI_setNextHoverCursor(OS_CursorType value) { StackSetNextImpl(uiContext, HoverCursor, hoverCursor, value) }

f32 UI_topOpacity() { StackTopImpl(uiContext, Opacity, opacity)}
f32 UI_pushOpacity(f32 value) { StackPushImpl(uiContext, Opacity, opacity, value) }
f32 UI_popOpacity() { StackPopImpl(uiContext, Opacity, opacity) }
f32 UI_setNextOpacity(f32 value) { StackSetNextImpl(uiContext, Opacity, opacity, value) }

// Text decorations
UI_TextAlignment UI_topTextAlignment() { StackTopImpl(uiContext, TextAlignment, textAlignment)}
UI_TextAlignment UI_pushTextAlignment(UI_TextAlignment value) { StackPushImpl(uiContext, TextAlignment, textAlignment, value) }
UI_TextAlignment UI_popTextAlignment() { StackPopImpl(uiContext, TextAlignment, textAlignment) }
UI_TextAlignment UI_setNextTextAlignment(UI_TextAlignment value) { StackSetNextImpl(uiContext, TextAlignment, textAlignment, value) }

f32 UI_topTextEdgePadding() { StackTopImpl(uiContext, TextEdgePadding, textEdgePadding)}
f32 UI_pushTextEdgePadding(f32 value) { StackPushImpl(uiContext, TextEdgePadding, textEdgePadding, value) }
f32 UI_popTextEdgePadding() { StackPopImpl(uiContext, TextEdgePadding, textEdgePadding) }
f32 UI_setNextTextEdgePadding(f32 value) { StackSetNextImpl(uiContext, TextEdgePadding, textEdgePadding, value) }

FontAsset* UI_topFont() { StackTopImpl(uiContext, Font, font)}
FontAsset* UI_pushFont(FontAsset* value) { StackPushImpl(uiContext, Font, font, value) }
FontAsset* UI_popFont() { StackPopImpl(uiContext, Font, font) }
FontAsset* UI_setNextFont(FontAsset* value) { StackSetNextImpl(uiContext, Font, font, value) }

f32 UI_topFontSize() { StackTopImpl(uiContext, FontSize, fontSize)}
f32 UI_pushFontSize(f32 value) { StackPushImpl(uiContext, FontSize, fontSize, value) }
f32 UI_popFontSize() { StackPopImpl(uiContext, FontSize, fontSize) }
f32 UI_setNextFontSize(f32 value) { StackSetNextImpl(uiContext, FontSize, fontSize, value) }

vec4 UI_topTextColor() { StackTopImpl(uiContext, TextColor, textColor)}
vec4 UI_pushTextColor(vec4 value) { StackPushImpl(uiContext, TextColor, textColor, value) }
vec4 UI_popTextColor() { StackPopImpl(uiContext, TextColor, textColor) }
vec4 UI_setNextTextColor(vec4 value) { StackSetNextImpl(uiContext, TextColor, textColor, value) }

// Rect decorations
f32 UI_topCornerRadius00() { StackTopImpl(uiContext, CornerRadius00, cornerRadius00)}
f32 UI_pushCornerRadius00(f32 value) { StackPushImpl(uiContext, CornerRadius00, cornerRadius00, value) }
f32 UI_popCornerRadius00() { StackPopImpl(uiContext, CornerRadius00, cornerRadius00) }
f32 UI_setNextCornerRadius00(f32 value) { StackSetNextImpl(uiContext, CornerRadius00, cornerRadius00, value) }

f32 UI_topCornerRadius01() { StackTopImpl(uiContext, CornerRadius01, cornerRadius01)}
f32 UI_pushCornerRadius01(f32 value) { StackPushImpl(uiContext, CornerRadius01, cornerRadius01, value) }
f32 UI_popCornerRadius01() { StackPopImpl(uiContext, CornerRadius01, cornerRadius01) }
f32 UI_setNextCornerRadius01(f32 value) { StackSetNextImpl(uiContext, CornerRadius01, cornerRadius01, value) }

f32 UI_topCornerRadius10() { StackTopImpl(uiContext, CornerRadius10, cornerRadius10)}
f32 UI_pushCornerRadius10(f32 value) { StackPushImpl(uiContext, CornerRadius10, cornerRadius10, value) }
f32 UI_popCornerRadius10() { StackPopImpl(uiContext, CornerRadius10, cornerRadius10) }
f32 UI_setNextCornerRadius10(f32 value) { StackSetNextImpl(uiContext, CornerRadius10, cornerRadius10, value) }

f32 UI_topCornerRadius11() { StackTopImpl(uiContext, CornerRadius11, cornerRadius11)}
f32 UI_pushCornerRadius11(f32 value) { StackPushImpl(uiContext, CornerRadius11, cornerRadius11, value) }
f32 UI_popCornerRadius11() { StackPopImpl(uiContext, CornerRadius11, cornerRadius11) }
f32 UI_setNextCornerRadius11(f32 value) { StackSetNextImpl(uiContext, CornerRadius11, cornerRadius11, value) }

f32 UI_topBorderThickness() { StackTopImpl(uiContext, BorderThickness, borderThickness)}
f32 UI_pushBorderThickness(f32 value) { StackPushImpl(uiContext, BorderThickness, borderThickness, value) }
f32 UI_popBorderThickness() { StackPopImpl(uiContext, BorderThickness, borderThickness) }
f32 UI_setNextBorderThickness(f32 value) { StackSetNextImpl(uiContext, BorderThickness, borderThickness, value) }

vec4 UI_topBackgroundColor() { StackTopImpl(uiContext, BackgroundColor, backgroundColor)}
vec4 UI_pushBackgroundColor(vec4 value) { StackPushImpl(uiContext, BackgroundColor, backgroundColor, value) }
vec4 UI_popBackgroundColor() { StackPopImpl(uiContext, BackgroundColor, backgroundColor) }
vec4 UI_setNextBackgroundColor(vec4 value) { StackSetNextImpl(uiContext, BackgroundColor, backgroundColor, value) }

vec4 UI_topBorderColor() { StackTopImpl(uiContext, BorderColor, borderColor)}
vec4 UI_pushBorderColor(vec4 value) { StackPushImpl(uiContext, BorderColor, borderColor, value) }
vec4 UI_popBorderColor() { StackPopImpl(uiContext, BorderColor, borderColor) }
vec4 UI_setNextBorderColor(vec4 value) { StackSetNextImpl(uiContext, BorderColor, borderColor, value) }

vec4 UI_topOverlayColor() { StackTopImpl(uiContext, OverlayColor, overlayColor)}
vec4 UI_pushOverlayColor(vec4 value) { StackPushImpl(uiContext, OverlayColor, overlayColor, value) }
vec4 UI_popOverlayColor() { StackPopImpl(uiContext, OverlayColor, overlayColor) }
vec4 UI_setNextOverlayColor(vec4 value) { StackSetNextImpl(uiContext, OverlayColor, overlayColor, value) }

vec4 UI_topFillColor() { StackTopImpl(uiContext, FillColor, fillColor)}
vec4 UI_pushFillColor(vec4 value) { StackPushImpl(uiContext, FillColor, fillColor, value) }
vec4 UI_popFillColor() { StackPopImpl(uiContext, FillColor, fillColor) }
vec4 UI_setNextFillColor(vec4 value) { StackSetNextImpl(uiContext, FillColor, fillColor, value) }

#undef StackPushImpl
#undef StackSetNextImpl

}// namespace pm
