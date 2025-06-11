#pragma once

#include "core/core.h"
#include "core/math/math.h"

namespace pm {


using MatchFlags = u32;
enum {
	MatchFlag_CaseInsensitive = (1 << 0),
	MatchFlag_RightSideSloppy = (1 << 1),
	MatchFlag_SlashInsensitive = (1 << 2),
	MatchFlag_FindLast = (1 << 3),
	MatchFlag_KeepEmpties = (1 << 4),
};

b32 charIsAlpha(u8 c);
b32 charIsAlphaUpper(u8 c);
b32 charIsAlphaLower(u8 c);
b32 charIsDigit(u8 c);
b32 charIsSymbol(u8 c);
b32 charIsSpace(u8 c);
u8 charToUpper(u8 c);
u8 charToLower(u8 c);
u8 charToForwardSlash(u8 c);

u64 calculateCStringLength(char* str);

String8 Str8(u8* str, u64 size);

#define Str8Zero() Str8{ 0, 0 }
#define Str8C(cstring) \
	Str8 { (u8*)(cstring), calculateCStringLength(cstring) }
#define Str8L(s) Str8((u8*)(s), sizeof(s) - 1)

String8 Str8Range(u8* first, u8* onePastLast);
String16 Str16(u16* str, u64 size);
String32 Str32(u32* str, u64 size);

// Arena functionality
String8 PushStr8Copy(Arena* arena, String8 string);
String8 PushStr8FV(Arena* arena, char* fmt, va_list args);
String8 PushStr8F(Arena* arena, char* fmt, ...);
String8 PushStr8FillByte(Arena* arena, u64 size, u8 byte);

// matching
b32 Str8Match(String8 a, String8 b, MatchFlags flags);

String8 Str8Skip(String8 str, u64 min);

// Substrings
String8 Substr8(String8 str, Rect1D rect);
u64 FindSubstr8(String8 haystack, String8 needle, u64 startPos, MatchFlags flags);

}// namespace pm
