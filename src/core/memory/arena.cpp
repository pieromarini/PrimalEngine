#include "arena.h"
#include "core/core.h"
#include "platform/os/os.h"
#include <cassert>


namespace pm {

Arena* arenaAlloc(u64 size) {
	return arenaAlloc({ .flags = arenaDefaultFlags, .reserveSize = size, .commitSize = arenaDefaultCommitSize });
}

Arena* arenaAlloc(ArenaParams params) {
	auto reserveSize = params.reserveSize;
	auto commitSize = params.commitSize;

	reserveSize = AlignPow2(reserveSize, OS_pageSize());
	commitSize = AlignPow2(commitSize, OS_pageSize());

	void* base = params.optionalBackingBuffer;
	if (base == nullptr) {
		base = OS_reserve(reserveSize);
		OS_commit(base, commitSize);
	}

	if (base == nullptr) {
		OS_abort();
	}

	auto* arena = (Arena*)base;
	arena->current = arena;
	arena->flags = params.flags;
	arena->nameSize = params.name.size;
	arena->commitedSize = (u32)params.commitSize;
	arena->reservedSize = params.reserveSize;
	arena->basePos = 0;
	arena->pos = ARENA_HEADER_SIZE;
	arena->commited = commitSize;
	arena->reserved = reserveSize;

	if (arena->nameSize != 0) {
		auto namePtr = (u8*)arenaPush(arena, arena->nameSize, 1);
		MemoryCopy(namePtr, params.name.str, Min(arena->nameSize, params.name.size));
		arenaPush(arena, 0, 128);
	}

	return arena;
}
void arenaRelease(Arena* arena) {
	for (Arena *a = arena->current, *prev = nullptr; a != nullptr; a = prev) {
		prev = a->prev;
		OS_release(a, a->reserved);
	}
}

String8 arenaName(Arena* arena) {
	String8 result = { .str = nullptr };
	result.size = (u64)arena->nameSize;
	if (result.size != 0) {
		result.str = (u8*)arena + ARENA_HEADER_SIZE;
	}
	return result;
}

void* arenaPush(Arena* arena, u64 size, u64 align) {
	Arena* current = arena->current;
	u64 lastPos = AlignPow2(current->pos, align);
	u64 newPos = lastPos + size;

	// TODO(piero): Implement chaining logic

	// commit memory if needed
	if (current->commited < newPos) {
		auto commitAligned = newPos + current->commitedSize - 1;
		commitAligned -= commitAligned % current->commitedSize;
		auto commitClamped = ClampTop(commitAligned, current->reserved);
		auto commitSize = commitClamped - current->commited;

		u8* commitPtr = (u8*)current + current->commited;
		OS_commit(commitPtr, commitSize);

		current->commited = commitClamped;
	}

	void* result = nullptr;
	if (current->commited >= newPos) {
		result = (u8*)current + lastPos;
		current->pos = newPos;
	}

	if (result == nullptr) {
		OS_abort();
	}

	return result;
}

u64 arenaPos(Arena* arena) {
	Arena* current = arena->current;
	u64 pos = current->basePos + current->pos;
	return pos;
}

void arenaPopTo(Arena* arena, u64 pos) {
	u64 big_pos = (ARENA_HEADER_SIZE > pos) ? ARENA_HEADER_SIZE : pos;
	Arena* current = arena->current;
	for (Arena* prev = nullptr; current->basePos >= big_pos; current = prev) {
		prev = current->prev;
		OS_release(current, current->reserved);
	}
	arena->current = current;
	u64 new_pos = big_pos - current->basePos;
	assert(new_pos <= current->pos);
	current->pos = new_pos;
}

void arenaClear(Arena* arena) {
	arenaPopTo(arena, 0);
}

void arenaPop(Arena* arena, u64 amt) {
	u64 pos_old = arenaPos(arena);
	u64 pos_new = pos_old;
	if (amt < pos_old) {
		pos_new = pos_old - amt;
	}
	arenaPopTo(arena, pos_new);
}

Temp tempBegin(Arena* arena) {
	u64 pos = arenaPos(arena);
	Temp temp = { .arena = arena, .pos = pos };
	return temp;
}

void tempEnd(Temp temp) {
	arenaPopTo(temp.arena, temp.pos);
}


}// namespace pm
