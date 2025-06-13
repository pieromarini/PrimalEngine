#pragma once

#include "ui/ui_types.h"

namespace pm {

void UI_Spacer(UI_Size size);

UI_Signal UI_Label(String8 string);
UI_Signal UI_LabelF(char *fmt, ...);

UI_Signal UI_Button(String8 string);

};
