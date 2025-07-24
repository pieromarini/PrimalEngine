#include "thread_context.h"

namespace pm {

per_thread ThreadCtx* threadCtx = 0;

ThreadCtx ThreadCtx_alloc() {
	ThreadCtx tctx = { .scratchArenas = { nullptr } };
	for (u64 arena_idx = 0; arena_idx < ArrayCount(tctx.scratchArenas); arena_idx += 1) {
		tctx.scratchArenas[arena_idx] = ArenaAllocDefault();
	}
	return tctx;
}

void ThreadCtx_release() {
	for (u64 arena_idx = 0; arena_idx < ArrayCount(threadCtx->scratchArenas); arena_idx += 1) {
		arenaRelease(threadCtx->scratchArenas[arena_idx]);
	}
}

void ThreadCtx_set(ThreadCtx* ctx) {
	threadCtx = ctx;
}
ThreadCtx* ThreadCtx_get() {
	return threadCtx;
}

void ThreadCtx_setName(String8 string) {
}
void ThreadCtx_getName() {
}
b32 ThreadCtx_isMainThread() {
	return threadCtx->isMainThread;
}

Temp ScratchBegin(Arena** conflicts, u64 conflictsCount) {
	Temp scratch = { .arena = nullptr };
	auto* tctx = ThreadCtx_get();
	for (u64 tctx_idx = 0; tctx_idx < ArrayCount(tctx->scratchArenas); tctx_idx += 1) {
		b32 is_conflicting = 0;
		for (Arena** conflict = conflicts; conflict < conflicts + conflictsCount; conflict += 1) {
			if (*conflict == tctx->scratchArenas[tctx_idx]) {
				is_conflicting = 1;
				break;
			}
		}
		if (is_conflicting == 0) {
			scratch.arena = tctx->scratchArenas[tctx_idx];
			scratch.pos = scratch.arena->pos;
			break;
		}
	}
	return scratch;
}

};// namespace pm
