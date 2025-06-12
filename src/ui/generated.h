#pragma once

#include "assets/asset.h"
#include "core/data_structures/stack.h"
#include "ui_types.h"

namespace pm {

StackDeclareNodeWithPointer(Parent, UIElement);
StackDeclareNode(PrefDim, UI_Size);
StackDeclareNode(ChildLayoutAxis, Axis2D);
StackDeclareNode(FixedX, f32);
StackDeclareNode(FixedY, f32);
StackDeclareNode(SeedKey, UIKey);
StackDeclareNode(Flags, UI_ElementFlags);

StackDeclareNode(FocusHot, UI_FocusKind);
StackDeclareNode(FocusActive, UI_FocusKind);

StackDeclareNode(HoverCursor, u64);
StackDeclareNode(Opacity, f32);

// Text decorations
StackDeclareNode(TextAlignment, UI_TextAlignment);
StackDeclareNode(TextEdgePadding, f32);
StackDeclareNode(FontSize, f32);
StackDeclareNodeWithPointer(Font, FontAsset);
StackDeclareNode(TextColor, vec4);

// Rect decorations
StackDeclareNode(CornerRadius00, f32);
StackDeclareNode(CornerRadius01, f32);
StackDeclareNode(CornerRadius10, f32);
StackDeclareNode(CornerRadius11, f32);
StackDeclareNode(BorderThickness, f32);
StackDeclareNode(BackgroundColor, vec4);
StackDeclareNode(BorderColor, vec4);
StackDeclareNode(OverlayColor, vec4);

UIElement* UI_topParent();
UIElement* UI_pushParent(UIElement* value);
UIElement* UI_popParent();
UIElement* UI_setNextParent(UIElement* value);

UI_Size UI_topPrefWidth();
UI_Size UI_pushPrefWidth(UI_Size value);
UI_Size UI_popPrefWidth();
UI_Size UI_setNextPrefWidth(UI_Size value);

UI_Size UI_topPrefHeight();
UI_Size UI_pushPrefHeight(UI_Size value);
UI_Size UI_popPrefHeight();
UI_Size UI_setNextPrefHeight(UI_Size value);

Axis2D UI_topChildLayoutAxis();
Axis2D UI_pushChildLayoutAxis(Axis2D value);
Axis2D UI_popChildLayoutAxis();
Axis2D UI_setNextChildLayoutAxis(Axis2D value);

f32 UI_topFixedX();
f32 UI_pushFixedX(f32 value);
f32 UI_popFixedX();
f32 UI_setNextFixedX(f32 value);

f32 UI_topFixedY();
f32 UI_pushFixedY(f32 value);
f32 UI_popFixedY();
f32 UI_setNextFixedY(f32 value);

UIKey UI_topSeedKey();
UIKey UI_pushSeedKey(UIKey value);
UIKey UI_popSeedKey();
UIKey UI_setNextSeedKey(UIKey value);

UI_ElementFlags UI_topFlags();
UI_ElementFlags UI_pushFlags(UI_ElementFlags value);
UI_ElementFlags UI_popFlags();
UI_ElementFlags UI_setNextFlags(UI_ElementFlags value);

UI_FocusKind UI_topFocusHot();
UI_FocusKind UI_pushFocusHot(UI_FocusKind value);
UI_FocusKind UI_popFocusHot();
UI_FocusKind UI_setNextFocusHot(UI_FocusKind value);

UI_FocusKind UI_topFocusActive();
UI_FocusKind UI_pushFocusActive(UI_FocusKind value);
UI_FocusKind UI_popFocusActive();
UI_FocusKind UI_setNextFocusActive(UI_FocusKind value);

u64 UI_topHoverCursor();
u64 UI_pushHoverCursor(u64 value);
u64 UI_popHoverCursor();
u64 UI_setNextHoverCursor(u64 value);

f32 UI_topOpacity();
f32 UI_pushOpacity(f32 value);
f32 UI_popOpacity();
f32 UI_setNextOpacity(f32 value);

// Text decorations
UI_TextAlignment UI_topTextAlignment();
UI_TextAlignment UI_pushTextAlignment(UI_TextAlignment value);
UI_TextAlignment UI_popTextAlignment();
UI_TextAlignment UI_setNextTextAlignment(UI_TextAlignment value);

f32 UI_topTextEdgePadding();
f32 UI_pushTextEdgePadding(f32 value);
f32 UI_popTextEdgePadding();
f32 UI_setNextTextEdgePadding(f32 value);

FontAsset* UI_topFont();
FontAsset* UI_pushFont(FontAsset* value);
FontAsset* UI_popFont();
FontAsset* UI_setNextFont(FontAsset* value);

f32 UI_topFontSize();
f32 UI_pushFontSize(f32 value);
f32 UI_popFontSize();
f32 UI_setNextFontSize(f32 value);

vec4 UI_topTextColor();
vec4 UI_pushTextColor(vec4 value);
vec4 UI_popTextColor();
vec4 UI_setNextTextColor(vec4 value);

// Rect decorations
f32 UI_topCornerRadius00();
f32 UI_pushCornerRadius00(f32 value);
f32 UI_popCornerRadius00();
f32 UI_setNextCornerRadius00(f32 value);

f32 UI_topCornerRadius01();
f32 UI_pushCornerRadius01(f32 value);
f32 UI_popCornerRadius01();
f32 UI_setNextCornerRadius01(f32 value);

f32 UI_topCornerRadius10();
f32 UI_pushCornerRadius10(f32 value);
f32 UI_popCornerRadius10();
f32 UI_setNextCornerRadius10(f32 value);

f32 UI_topCornerRadius11();
f32 UI_pushCornerRadius11(f32 value);
f32 UI_popCornerRadius11();
f32 UI_setNextCornerRadius11(f32 value);

f32 UI_topBorderThickness();
f32 UI_pushBorderThickness(f32 value);
f32 UI_popBorderThickness();
f32 UI_setNextBorderThickness(f32 value);

vec4 UI_topBackgroundColor();
vec4 UI_pushBackgroundColor(vec4 value);
vec4 UI_popBackgroundColor();
vec4 UI_setNextBackgroundColor(vec4 value);

vec4 UI_topBorderColor();
vec4 UI_pushBorderColor(vec4 value);
vec4 UI_popBorderColor();
vec4 UI_setNextBorderColor(vec4 value);

vec4 UI_topOverlayColor();
vec4 UI_pushOverlayColor(vec4 value);
vec4 UI_popOverlayColor();
vec4 UI_setNextOverlayColor(vec4 value);


}// namespace pm
