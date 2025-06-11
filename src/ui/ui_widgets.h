#pragma once

#include "ui/ui_types.h"

namespace pm {

struct UI_SliderF32DrawData {
 f32 percentageFilled;
 vec4 fillColor;
};

struct UI_LineEditDrawData {
 Rect1DF32 selectionRangePx;
};

UI_Signal UI_Label(String8 string);
UI_Signal UI_LabelF(char *fmt, ...);

UI_Signal UI_Button(String8 string);

};
