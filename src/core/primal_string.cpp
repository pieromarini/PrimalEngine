#include "primal_string.h"
#include "core/math/math.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#include <cstdarg>

namespace pm {

b32 charIsAlpha(u8 c) {
	return charIsAlphaUpper(c) || charIsAlphaLower(c);
}

b32 charIsAlphaUpper(u8 c) {
	return c >= 'A' && c <= 'Z';
}

b32 charIsAlphaLower(u8 c) {
	return c >= 'a' && c <= 'z';
}

b32 charIsDigit(u8 c) {
	return (c >= '0' && c <= '9');
}

b32 charIsSymbol(u8 c) {
	return (c == '~' || c == '!' || c == '$' || c == '%' || c == '^' || c == '&' || c == '*' || c == '-' || c == '=' || c == '+' || c == '<' || c == '.' || c == '>' || c == '/' || c == '?' || c == '|' || c == '\\' || c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == '#' || c == ',' || c == ';' || c == ':' || c == '@');
}

b32 charIsSpace(u8 c) {
	return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v' || c == '\n';
}

u8 charToUpper(u8 c) {
	return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}

u8 charToLower(u8 c) {
	return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

u8 charToForwardSlash(u8 c) {
	return (c == '\\' ? '/' : c);
}

u64 calculateCStringLength(char* str) {
	u64 length = 0;
	for (; str[length]; length += 1);
	return length;
}

String8 Str8(u8* str, u64 size) {
	String8 string{};
	string.str = str;
	string.size = size;
	return string;
}

String8 Str8Range(u8* first, u8* one_past_last) {
	String8 string{};
	string.str = first;
	string.size = (u64)(one_past_last - first);
	return string;
}

String16 Str16(u16* str, u64 size) {
	String16 result{};
	result.str = str;
	result.size = size;
	return result;
}

String32 Str32(u32* str, u64 size) {
	String32 string = { .str = nullptr };
	string.str = str;
	string.size = size;
	return string;
}

String8 PushStr8Copy(Arena* arena, String8 string) {
	String8 res{};
	res.size = string.size;
	res.str = PushArrayNoZero(arena, u8, string.size + 1);
	MemoryCopy(res.str, string.str, string.size);
	res.str[string.size] = 0;
	return res;
}

String8 PushStr8FV(Arena* arena, char* fmt, va_list args) {
	String8 result = { .str = nullptr };
	va_list args2 = nullptr;
	va_copy(args2, args);
	u64 neededBytes = stbsp_vsnprintf(nullptr, 0, fmt, args) + 1;
	result.str = PushArrayNoZero(arena, u8, neededBytes);
	result.size = neededBytes - 1;
	stbsp_vsnprintf((char*)result.str, neededBytes, fmt, args2);
	return result;
}

String8 PushStr8F(Arena* arena, char* fmt, ...) {
	String8 result = { .str = nullptr };
	va_list args = nullptr;
	va_start(args, fmt);
	result = PushStr8FV(arena, fmt, args);
	va_end(args);
	return result;
}

String8 PushStr8FillByte(Arena* arena, u64 size, u8 byte) {
	String8 result = { .str = nullptr };
	result.str = PushArrayNoZero(arena, u8, size);
	MemorySet(result.str, byte, size);
	result.size = size;
	return result;
}

String8 Str8Skip(String8 str, u64 min) {
	return Substr8(str, Rect1D(min, str.size));
}

b32 Str8Match(String8 a, String8 b, MatchFlags flags) {
	b32 result = 0;
	if (a.size == b.size || flags & MatchFlag_RightSideSloppy) {
		result = 1;
		for (u64 i = 0; i < a.size; i += 1) {
			b32 match = (a.str[i] == b.str[i]);
			if (flags & MatchFlag_CaseInsensitive) {
				match |= (charToLower(a.str[i]) == charToLower(b.str[i]));
			}
			if (flags & MatchFlag_SlashInsensitive) {
				match |= (charToForwardSlash(a.str[i]) == charToForwardSlash(b.str[i]));
			}
			if (match == 0) {
				result = 0;
				break;
			}
		}
	}
	return result;
}

String8 Substr8(String8 str, Rect1D rect) {
	auto min = rect.min;
	auto max = rect.max;
	if (max > str.size) {
		max = str.size;
	}
	if (min > str.size) {
		min = str.size;
	}
	if (min > max) {
		u64 swap = min;
		min = max;
		max = swap;
	}
	str.size = max - min;
	str.str += min;
	return str;
}

u64 FindSubstr8(String8 haystack, String8 needle, u64 startPos, MatchFlags flags) {
	b32 found = 0;
	u64 found_idx = haystack.size;
	for (u64 i = startPos; i < haystack.size; i += 1) {
		if (i + needle.size <= haystack.size) {
			String8 substr = Substr8(haystack, Rect1D(i, i + needle.size));
			if (Str8Match(substr, needle, flags)) {
				found_idx = i;
				found = 1;
				if (!(flags & MatchFlag_FindLast)) {
					break;
				}
			}
		}
	}
	return found_idx;
}

};// namespace pm
