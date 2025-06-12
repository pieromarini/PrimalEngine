#pragma once

#include "core/core.h"
#include "core/math/math.h"
#include "core/primal_string.h"

#include "keys.h"

namespace pm {

using OS_Handle = u64;

enum OS_EventKind {
	OSEventKind_Null,
	OSEventKind_WindowClose,
	OSEventKind_WindowLoseFocus,
	OSEventKind_Press,
	OSEventKind_Release,
	OSEventKind_Text,
	OSEventKind_Scroll,
	OSEventKind_DropFile,
	OSEventKind_COUNT
};

using OS_Modifiers = u32;
enum {
	OS_Modifier_Ctrl = (1 << 0),
	OS_Modifier_Shift = (1 << 1),
	OS_Modifier_Alt = (1 << 2),
};

struct OS_Event {
	OS_Event* next;
	OS_Event* prev;
	OS_Handle window;
	OS_EventKind kind;
	OS_Modifiers modifiers;
	OS_Key key;
	u32 character;
	vec2 position;
	vec2 scroll;
	String8 path;
};

struct OS_EventList {
	OS_Event* first;
	OS_Event* last;
	u64 count;
};

struct OS_ModifiersKeyPair {
	OS_Modifiers modifiers;
	OS_Key key;
};

}// namespace pm
