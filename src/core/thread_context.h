#pragma once

#include "core/memory/arena.h"

namespace pm {

struct Arena;
struct Temp;

struct ThreadCtx {
	Arena* scratchArenas[2];
	u8 threadName[64];
	u64 threadNameSize;
	bool isMainThread;
};

ThreadCtx ThreadCtx_alloc();
void ThreadCtx_release();

void ThreadCtx_set(ThreadCtx* ctx);
ThreadCtx* ThreadCtx_get();

void ThreadCtx_setName(String8 string);
void ThreadCtx_getName();
b32 ThreadCtx_isMainThread();

Temp ScratchBegin(Arena** conflicts = nullptr, u64 conflictsCount = 0);
#define ScratchEnd(temp) tempEnd(temp);

};// namespace pm
