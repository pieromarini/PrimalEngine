#pragma once

#include "core/core.h"

namespace pm {

#define StackDeclareNode(node_name, type) \
	struct node_name##Node {                \
		node_name##Node* next;                \
		type value;                           \
	};

#define StackDeclareNodeWithPointer(struct_name, type) \
	struct struct_name##Node {                           \
		struct_name##Node* next;                           \
		type* value;                                       \
	};

// NOTE(piero): Declare a stack and a nil node to be used as a default instead of nullptr
#define StackDeclare(node_name, struct_name) \
	node_name##Node struct_name##Nil;          \
	struct {                                   \
		node_name##Node* top;                    \
		node_name##Node* free;                   \
		bool autoPop;                            \
	} struct_name##Stack

#define StackInitNils(state, struct_name, default_value) state->struct_name##Nil = { .next = nullptr, .value = (default_value) };

// NOTE(piero): Create stacks using initializer list.
#define StackCreate(context, struct_name) { .top = &context->struct_name##Nil, .free = nullptr }

#define StackTopImpl(state, name_upper, name_lower) \
	return state->name_lower##Stack.top->value;

#define StackPushImplArena(state, name_upper, name_lower, new_value, arena) \
	name_upper##Node* node = state->name_lower##Stack.free;       \
	if (node != 0) {                                              \
		StackPop(state->name_lower##Stack.free);                    \
	} else {                                                      \
		node = PushStruct((arena), name_upper##Node);       \
	}                                                             \
	auto old_value = state->name_lower##Stack.top->value;         \
	node->value = new_value;                                      \
	StackPush(state->name_lower##Stack.top, node);                \
	return old_value;

#define StackPopImpl(state, name_upper, name_lower)        \
	name_upper##Node* popped = state->name_lower##Stack.top; \
	if (popped != &state->name_lower##Nil) {                 \
		StackPop(state->name_lower##Stack.top);                \
		StackPush(state->name_lower##Stack.free, popped);      \
	}                                                        \
	return popped->value;

#define StackSetNextImplArena(state, name_upper, name_lower, new_value, arena) \
	name_upper##Node* node = state->name_lower##Stack.free;          \
	if (node != 0) {                                                 \
		StackPop(state->name_lower##Stack.free);                       \
	} else {                                                         \
		node = PushStruct((arena), name_upper##Node);          \
	}                                                                \
	auto old_value = state->name_lower##Stack.top->value;            \
	node->value = new_value;                                         \
	StackPush(state->name_lower##Stack.top, node);                   \
	state->name_lower##Stack.autoPop = 1;                            \
	return old_value;

}// namespace pm
