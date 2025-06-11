#pragma once

#include "SDL3/SDL_mouse.h"
#include "core/core.h"
#include "core/math/math.h"

#if defined(PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
// NOTE(piero): These conflict with our Min/Max macros
#define NOMINMAX
#include <windows.h>
#elif defined(PLATFORM_POSIX)
#include <sys/mman.h>
#include <unistd.h>
#endif


namespace pm {

// TODO(piero): Implement for linux

inline u64 OS_pageSize() {
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return info.dwPageSize;
}

inline void* OS_reserve(u64 size) {
	u64 gbSize = size;
	gbSize += Gigabytes(1) - 1;
	gbSize -= gbSize % Gigabytes(1);
	void* ptr = VirtualAlloc(nullptr, gbSize, MEM_RESERVE, PAGE_NOACCESS);
	return ptr;
}

inline void OS_release(void* ptr, u64 size) {
	VirtualFree(ptr, size, MEM_RELEASE);
}

inline void OS_commit(void* ptr, u64 size) {
	u64 pageAlignedSize = size;
	pageAlignedSize += OS_pageSize() - 1;
	pageAlignedSize -= pageAlignedSize % OS_pageSize();
	VirtualAlloc(ptr, pageAlignedSize, MEM_COMMIT, PAGE_READWRITE);
}

inline void OS_decommit(void* ptr, u64 size) {
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

inline void OS_abort() {
	ExitProcess(1);
}

inline vec2 OS_mouseFromWindow() {
	vec2 res{};
	SDL_GetGlobalMouseState(&res.x, &res.y);
	return res;
}

}// namespace pm
