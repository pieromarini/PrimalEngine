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

// NOTE(piero): This enum just casts directly to SDL's one.
enum OS_CursorType {
	OS_SYSTEM_CURSOR_DEFAULT,      /**< Default cursor. Usually an arrow. */
	OS_SYSTEM_CURSOR_TEXT,         /**< Text selection. Usually an I-beam. */
	OS_SYSTEM_CURSOR_WAIT,         /**< Wait. Usually an hourglass or watch or spinning ball. */
	OS_SYSTEM_CURSOR_CROSSHAIR,    /**< Crosshair. */
	OS_SYSTEM_CURSOR_PROGRESS,     /**< Program is busy but still interactive. Usually it's WAIT with an arrow. */
	OS_SYSTEM_CURSOR_NWSE_RESIZE,  /**< Double arrow pointing northwest and southeast. */
	OS_SYSTEM_CURSOR_NESW_RESIZE,  /**< Double arrow pointing northeast and southwest. */
	OS_SYSTEM_CURSOR_EW_RESIZE,    /**< Double arrow pointing west and east. */
	OS_SYSTEM_CURSOR_NS_RESIZE,    /**< Double arrow pointing north and south. */
	OS_SYSTEM_CURSOR_MOVE,         /**< Four pointed arrow pointing north, south, east, and west. */
	OS_SYSTEM_CURSOR_NOT_ALLOWED,  /**< Not permitted. Usually a slashed circle or crossbones. */
	OS_SYSTEM_CURSOR_POINTER,      /**< Pointer that indicates a link. Usually a pointing hand. */
	OS_SYSTEM_CURSOR_NW_RESIZE,    /**< Window resize top-left. This may be a single arrow or a double arrow like NWSE_RESIZE. */
	OS_SYSTEM_CURSOR_N_RESIZE,     /**< Window resize top. May be NS_RESIZE. */
	OS_SYSTEM_CURSOR_NE_RESIZE,    /**< Window resize top-right. May be NESW_RESIZE. */
	OS_SYSTEM_CURSOR_E_RESIZE,     /**< Window resize right. May be EW_RESIZE. */
	OS_SYSTEM_CURSOR_SE_RESIZE,    /**< Window resize bottom-right. May be NWSE_RESIZE. */
	OS_SYSTEM_CURSOR_S_RESIZE,     /**< Window resize bottom. May be NS_RESIZE. */
	OS_SYSTEM_CURSOR_SW_RESIZE,    /**< Window resize bottom-left. May be NESW_RESIZE. */
	OS_SYSTEM_CURSOR_W_RESIZE,     /**< Window resize left. May be EW_RESIZE. */
	OS_SYSTEM_CURSOR_COUNT
};

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
	SDL_GetMouseState(&res.x, &res.y);
	return res;
}

inline void OS_setCursor(OS_CursorType systemCursor) {
	auto cursorId = (SDL_SystemCursor)systemCursor;
	auto cursor = SDL_CreateSystemCursor(cursorId);
	SDL_SetCursor(cursor);

	// SDL_DestroyCursor(cursor);
}

}// namespace pm
