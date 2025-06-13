#pragma once

#include "core/core.h"
#include "core/memory/arena.h"
#include "platform/vulkan/vulkan_renderer.h"
#include "ui_types.h"
#include "core/data_structures/stack.h"
#include "core/primal_string.h"

#include "generated.h"

namespace pm {

struct UIElementSlot {
	UIElement* first;
	UIElement* last;
};

struct UIContext {
	Arena* arena;
	u64 buildGen;

	Arena* buildArenas[2];

	// interaction
	vec2 mouse;
	b32 actionKilledThisFrame;
	UIKey hotKey;
	UIKey activeKey[UIMouseButtonSlot_COUNT];
	vec2 dragStartMouse;
	Arena *dragDataArena;
	String8 dragData;
	
	// persistant UIElement state
	UIElement *firstFreeElement;
	u64 freeElementListCount;
	UIElementSlot *elementTable;
	u64 elementTableSize;

	// Per build params
	PrimalWindow* window;
	UI_EventList* events;
	UIElement* root;
	f32 deltaTime;

	// Stacks state
	// TODO(piero): Create a macro to generate these.
	StackDeclare(Parent, parent);
	StackDeclare(PrefDim, prefWidth);
	StackDeclare(PrefDim, prefHeight);
	StackDeclare(ChildLayoutAxis, childLayoutAxis);
	StackDeclare(FixedX, fixedX);
	StackDeclare(FixedY, fixedY);
	StackDeclare(SeedKey, seedKey);
	StackDeclare(Flags, flags);

	StackDeclare(FocusHot, focusHot);
	StackDeclare(FocusActive, focusActive);

	StackDeclare(HoverCursor, hoverCursor);
	StackDeclare(Opacity, opacity);

	// Text decorations
	StackDeclare(TextAlignment, textAlignment);
	StackDeclare(TextEdgePadding, textEdgePadding);
	StackDeclare(Font, font);
	StackDeclare(FontSize, fontSize);
	StackDeclare(TextColor, textColor);

	// Rect decorations
	StackDeclare(CornerRadius00, cornerRadius00);
	StackDeclare(CornerRadius01, cornerRadius01);
	StackDeclare(CornerRadius10, cornerRadius10);
	StackDeclare(CornerRadius11, cornerRadius11);
	StackDeclare(BorderThickness, borderThickness);
	StackDeclare(BackgroundColor, backgroundColor);
	StackDeclare(BorderColor, borderColor);
	StackDeclare(OverlayColor, overlayColor);
};

UIKey UI_keyZero();

void UI_setCurrentContext(UIContext* context);
UIContext* UI_currentContext();

UIContext* UI_createContext();
void UI_destroyContext(UIContext* context);

Arena* getBuildArena();

// tree traversal
UIElement_Rec UIElement_recurseDepthFirst(UIElement *element, UIElement *stopper, MemberOffset sib, MemberOffset child);
#define UIElement_recurseDepthFirstPre(box, stopper) UIElement_recurseDepthFirst((box), (stopper), MemberOff(UIElement, next), MemberOff(UIElement, first))
#define UIElement_recurseDepthFirstPost(box, stopper) UIElement_recurseDepthFirst((box), (stopper), MemberOff(UIElement, prev), MemberOff(UIElement, last))

// UI Element
bool UIElement_isNil(UIElement* element);
#define UIElement_setNil(b) ((b) = &nilUIElement)

UIElement* UIElement_create(UI_ElementFlags flags, char* fmt, ...);
UIElement* UIElement_create(UI_ElementFlags flags, String8 str);
UIElement* UIElement_createFromKey(UI_ElementFlags flags, UIKey key);
UIElement* UIElement_fromKey(UIKey key);

// events
OS_Key UI_OSKeyFromMouseButtonSlot(UI_MouseButtonSlot slot);
UI_MouseButtonSlot UI_mouseButtonSlotFromOSKey(OS_Key key);
void UI_eatEvent(UI_EventList *events, UI_Event *event);
b32 UI_keyPress(UI_EventList *events, OS_Key key, OS_Modifiers mods);
b32 UI_keyRelease(UI_EventList *events, OS_Key key, OS_Modifiers mods);
b32 UI_ctrlPress(UI_EventList *events, UI_CtrlSlot slot);
b32 UI_ctrlRelease(UI_EventList *events, UI_CtrlSlot slot);

// interactions
b32 UI_isFocusHot();
b32 UI_isFocusActive();
UI_Signal UI_signalFromElement(UIElement* element);

// layouts
void UI_layoutRoot(UIElement* root, Axis2D axis);
void UI_solveIndependentSizes(UIElement* root, Axis2D axis);
void UI_solveUpwardDependentSizes(UIElement* root, Axis2D axis);
void UI_solveDownwardDependentSizes(UIElement* root, Axis2D axis);
void UI_solveSizeViolations(UIElement* root, Axis2D axis);

// equips
void UIElement_EquipText(UIElement *element, String8 text);

// text
vec2 UI_textPosFromElement(UIElement* element);

// build scope
void UI_beginBuild(PrimalWindow* window, UI_EventList* events, f32 deltaTime);
void UI_endBuild();

void UI_draw(VulkanRendererContext* context);

// Extra stack helpers
void UI_pushCornerRadius(f32 v);
void UI_popCornerRadius();
void UI_setNextCornerRadius(f32 v);

void UI_pushPrefSize(Axis2D axis, UI_Size v);
void UI_popPrefSize(Axis2D axis);
void UI_setNextPrefSize(Axis2D axis, UI_Size v);

void UI_pushFixedPos(vec2 v);
void UI_popFixedPos();
void UI_setNextFixedPos(vec2 v);

void UI_pushFixedRect(Rect2D rect);
void UI_popFixedRect();
void UI_setNextFixedRect(Rect2D rect);

// NOTE(piero): Scope helpers
#define UI_parent(value) DeferLoop(UI_pushParent(value), UI_popParent())
#define UI_prefWidth(value) DeferLoop(UI_pushPrefWidth(value), UI_popPrefWidth())
#define UI_prefHeight(value) DeferLoop(UI_pushPrefHeight(value), UI_popPrefHeight())
#define UI_childLayoutAxis(value) DeferLoop(UI_pushChildLayoutAxis(value), UI_popChildLayoutAxis())
#define UI_seedKey(v) DeferLoop(UI_pushSeedKey(v), UI_popSeedKey())
#define UI_flags(value) DeferLoop(UI_pushFlags(value), UI_popFlags())

#define UI_textColor(value) DeferLoop(UI_pushTextColor(value), UI_popTextColor())
#define UI_textEdgePadding(value) DeferLoop(UI_pushTextEdgePadding(value), UI_popTextEdgePadding())

#define UI_autoPopStacks(state) \
if(state->parentStack.autoPop) { UI_popParent(); state->parentStack.autoPop = 0; }\
if(state->prefWidthStack.autoPop) { UI_popPrefWidth(); state->prefWidthStack.autoPop = 0; }\
if(state->prefHeightStack.autoPop) { UI_popPrefHeight(); state->prefHeightStack.autoPop = 0; }\
if(state->childLayoutAxisStack.autoPop) { UI_popChildLayoutAxis(); state->childLayoutAxisStack.autoPop = 0; }\
if(state->fixedXStack.autoPop) { UI_popFixedX(); state->fixedXStack.autoPop = 0; }\
if(state->fixedYStack.autoPop) { UI_popFixedY(); state->fixedYStack.autoPop = 0; }\
if(state->seedKeyStack.autoPop) { UI_popSeedKey(); state->seedKeyStack.autoPop = 0; }\
if(state->flagsStack.autoPop) { UI_popFlags(); state->flagsStack.autoPop = 0; }\
if(state->focusHotStack.autoPop) { UI_popFocusHot(); state->focusHotStack.autoPop = 0; }\
if(state->hoverCursorStack.autoPop) { UI_popHoverCursor(); state->hoverCursorStack.autoPop = 0; }\
if(state->opacityStack.autoPop) { UI_popOpacity(); state->opacityStack.autoPop = 0; }\
if(state->textAlignmentStack.autoPop) { UI_popTextAlignment(); state->textAlignmentStack.autoPop = 0; }\
if(state->textEdgePaddingStack.autoPop) { UI_popTextEdgePadding(); state->textEdgePaddingStack.autoPop = 0; }\
if(state->fontSizeStack.autoPop) { UI_popFontSize(); state->fontSizeStack.autoPop = 0; }\
if(state->fontStack.autoPop) { UI_popFont(); state->fontStack.autoPop = 0; }\
if(state->textColorStack.autoPop) { UI_popTextColor(); state->textColorStack.autoPop = 0; }\
if(state->cornerRadius00Stack.autoPop) { UI_popCornerRadius00(); state->cornerRadius00Stack.autoPop = 0; }\
if(state->cornerRadius01Stack.autoPop) { UI_popCornerRadius01(); state->cornerRadius01Stack.autoPop = 0; }\
if(state->cornerRadius10Stack.autoPop) { UI_popCornerRadius10(); state->cornerRadius10Stack.autoPop = 0; }\
if(state->cornerRadius11Stack.autoPop) { UI_popCornerRadius11(); state->cornerRadius11Stack.autoPop = 0; }\
if(state->borderThicknessStack.autoPop) { UI_popBorderThickness(); state->borderThicknessStack.autoPop = 0; }\
if(state->backgroundColorStack.autoPop) { UI_popBackgroundColor(); state->backgroundColorStack.autoPop = 0; }\
if(state->borderColorStack.autoPop) { UI_popBorderColor(); state->borderColorStack.autoPop = 0; }\
if(state->overlayColorStack.autoPop) { UI_popOverlayColor(); state->overlayColorStack.autoPop = 0; }

}// namespace pm:
