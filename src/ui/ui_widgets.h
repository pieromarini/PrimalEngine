#pragma once

#include "ui/ui_types.h"

namespace pm {

void UI_Spacer(UI_Size size);
#define UI_padding(size) DeferLoop(UI_Spacer(size), UI_Spacer(size))

UI_Signal UI_Label(String8 string);
UI_Signal UI_LabelF(const char* fmt, ...);

UI_Signal UI_Button(String8 string);

UI_Signal UI_Check(b32 checked, String8 string);
UI_Signal UI_CheckF(b32 checked, char *fmt, ...);

// Parent Layout helpers
void UI_namedColumnBegin(String8 string);
void UI_namedColumnBeginF(char *fmt, ...);
void UI_columnBegin();
void UI_columnEnd();
#define UI_namedColumn(s) DeferLoop(UI_namedColumnBegin(s), UI_columnEnd())
#define UI_namedColumnF(...) DeferLoop(UI_namedColumnBeginF(__VA_ARGS__), UI_columnEnd())
#define UI_column DeferLoop(UI_columnBegin(), UI_columnEnd())

void UI_namedRowBegin(String8 string);
void UI_namedRowBeginF(char *fmt, ...);
void UI_rowBegin();
void UI_rowEnd();
#define UI_namedRow(s) DeferLoop(UI_namedRowBegin(s), UI_rowEnd())
#define UI_namedRowF(...) DeferLoop(UI_namedRowBeginF(__VA_ARGS__), UI_rowEnd())
#define UI_row DeferLoop(UI_rowBegin(), UI_rowEnd())

};
