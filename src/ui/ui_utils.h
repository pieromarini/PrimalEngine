#pragma once

#include "core/core.h"
#include "core/primal_string.h"
#include "ui/ui_types.h"

namespace pm {

inline String8 UI_TextPartFromKeyString(String8 string) {
	u64 double_pound_pos = FindSubstr8(string, Str8L("##"), 0, 0);
	if (double_pound_pos < string.size) {
		string.size = double_pound_pos;
	}
	return string;
}

inline String8 UI_HashPartFromKeyString(String8 string) {
	u64 triple_pound_pos = FindSubstr8(string, Str8L("###"), 0, 0);
	if (triple_pound_pos < string.size) {
		string = Str8Skip(string, triple_pound_pos);
	}
	return string;
}

inline UIKey UI_KeyFromString(UIKey seed, String8 string) {
	UIKey key = { 0 };
	if (string.size != 0) {
		MemoryCopyStruct(&key, &seed);
		for (u64 i = 0; i < string.size; i += 1) {
			key.v[0] = ((key.v[0] << 5) + key.v[0]) + string.str[i];
		}
	}
	return key;
}

inline b32 UI_KeyMatch(UIKey a, UIKey b) {
	return (a.v[0] == b.v[0]);
}

};// namespace pm
