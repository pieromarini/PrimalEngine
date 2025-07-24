#pragma once

#include "core/memory/arena.h"
#include <cstddef>
#include <cstring>

namespace pm {

#define DynamicArrayDeclare(name, type) struct name { type* data; ptrdiff_t len; ptrdiff_t cap; }

#define DynamicArray_push(arena, s)                             						 \
	((s)->len >= (s)->cap                                         						 \
	 ? DynamicArray_grow(arena, s, sizeof(*(s)->data)), (s)->data + (s)->len++ \
	 : (s)->data + (s)->len++)

static void DynamicArray_grow(Arena* a, void *slice, std::ptrdiff_t size) {
	struct {
		void *data;
		ptrdiff_t len;
		ptrdiff_t cap;
	} replica{};
	memcpy(&replica, slice, sizeof(replica));

	replica.cap = replica.cap ? replica.cap : 1;
	ptrdiff_t align = 16;
	void *data = arenaPush(a, 2 * size * replica.cap, align);
	replica.cap *= 2;
	if (replica.len) {
		memcpy(data, replica.data, size * replica.len);
	}
	replica.data = data;

	memcpy(slice, &replica, sizeof(replica));
}

};// namespace pm
