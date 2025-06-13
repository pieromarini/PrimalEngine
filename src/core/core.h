#pragma once

#include "config.h"
#include <cstdint>

namespace pm {

// Custom base type names
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using b8 = i8;
using b16 = i16;
using b32 = i32;
using b64 = i64;
using f32 = float;
using f64 = double;

// integer/pointers
#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define IntFromPtr(p) (u64)(((u8*)p) - 0)
#define PtrFromInt(i) (void*)(((u8*)0) + i)
#define Member(type, member_name) ((type *)0)->member_name
#define OffsetOf(type, member_name) IntFromPtr(&Member(type, member_name))
#define BaseFromMember(type, member_name, ptr) (type *)((u8 *)(ptr) - OffsetOf(type, member_name))

// Units
#define Bytes(n) (n)
#define Kilobytes(n) (n << 10)
#define Megabytes(n) (n << 20)
#define Gigabytes(n) (((u64)n) << 30)
#define Terabytes(n) (((u64)n) << 40)

// Clamps, min, max
#define Min(a, b) (((a) < (b)) ? (a) : (b))
#define Max(a, b) (((a) > (b)) ? (a) : (b))
#define ClampTop(x, a) Min(x,a)
#define ClampBot(a, x) Max(a,x)
#define Clamp(a, x, b) (((a)>(x))?(a):((b)<(x))?(b):(x))

// Fake context
#define DeferLoop(start, end) for(int _i_ = ((start), 0); _i_ == 0; (_i_ += 1, (end)))

// Alignments
#define AlignPow2(x, b) (((x) + (b) - 1) & (~((b) - 1)))
#define AlignDownPow2(x, b) ((x) & (~((b) - 1)))

#if defined(_WIN32)
#define PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__APPLE__)
#define PLATFORM_POSIX
#endif

// assertions
#if defined(PLATFORM_WINDOWS)
#define BreakDebugger() __debugbreak()
#else
#define BreakDebugger() (*(volatile int *)0 = 0)
#endif

// #define AssertAlways(b) do { if(!(b)) { BreakDebugger(); } } while(0)

#undef Assert
#if DEBUG
# define Assert(b) do { if(!(b)) { BreakDebugger(); } } while(0)
#else
# define Assert(b) ((void)(b))
#endif

// #define NotImplemented AssertAlways(!"Not Implemented")
// #define InvalidPath AssertAlways(!"Invalid Path")

// memory copy/move/set wrappers
#define MemoryCopy(dst, src, size) memcpy((dst), (src), (size))
#define MemoryMove(dst, src, size) memmove((dst), (src), (size))
#define MemorySet(dst, byte, size) memset((dst), (byte), (size))

#define MemoryCopyStruct(dst, src)            \
	do {                                        \
		Assert(sizeof(*(dst)) == sizeof(*(src))); \
		MemoryCopy((dst), (src), sizeof(*(dst))); \
	} while (0)
#define MemoryCopyArray(dst, src)          \
	do {                                     \
		Assert(sizeof(dst) == sizeof(src));    \
		MemoryCopy((dst), (src), sizeof(src)); \
	} while (0)

#define MemoryZero(ptr, size) MemorySet((ptr), 0, (size))
#define MemoryZeroStruct(ptr) MemoryZero((ptr), sizeof(*(ptr)))
#define MemoryZeroArray(arr) MemoryZero((arr), sizeof(arr))

#if COMPILER_MSVC
#define per_thread __declspec(thread)
#elif COMPILER_CLANG || COMPILER_GCC
#define per_thread __thread
#else
#define per_thread
#endif

// NOTE(piero): These are used to create a "context" with braces.
//              Right now they are used in our UI code.
//              Sample usage: FakeContext(PushSomething, PopSomething) { do some stuff with whatever was pushed }
#define FakeContext(start, end) for (int _i_ = ((start), 0); _i_ == 0; (_i_ += 1, (end)))
#define FakeContextChecked(start, end) for(int _i_ = 2 * !(start); (_i_ == 2 ? ((end), 0) : !_i_); _i_ += 1, (end))


// Linked List helpers
// Based on: https://www.youtube.com/watch?v=gAijHHlyD5s
#define CheckNull(p) ((p)==0)
#define SetNull(p) ((p)=0)

#define QueuePush_NZ(f,l,n,next,zchk,zset) (zchk(f)?\
(((f)=(l)=(n)), zset((n)->next)):\
((l)->next=(n),(l)=(n),zset((n)->next)))
#define QueuePushFront_NZ(f,l,n,next,zchk,zset) (zchk(f) ? (((f) = (l) = (n)), zset((n)->next)) :\
((n)->next = (f)), ((f) = (n)))
#define QueuePop_NZ(f,l,next,zset) ((f)==(l)?\
(zset(f),zset(l)):\
((f)=(f)->next))
#define StackPush_N(f,n,next) ((n)->next=(f),(f)=(n))
#define StackPop_NZ(f,next,zchk) (zchk(f)?0:((f)=(f)->next))

#define DLLInsert_NPZ(f,l,p,n,next,prev,zchk,zset) \
(zchk(f) ? (((f) = (l) = (n)), zset((n)->next), zset((n)->prev)) :\
zchk(p) ? (zset((n)->prev), (n)->next = (f), (zchk(f) ? (0) : ((f)->prev = (n))), (f) = (n)) :\
((zchk((p)->next) ? (0) : (((p)->next->prev) = (n))), (n)->next = (p)->next, (n)->prev = (p), (p)->next = (n),\
((p) == (l) ? (l) = (n) : (0))))
#define DLLPushBack_NPZ(f,l,n,next,prev,zchk,zset) DLLInsert_NPZ(f,l,l,n,next,prev,zchk,zset)
#define DLLRemove_NPZ(f,l,n,next,prev,zchk,zset) (((f)==(n))?\
((f)=(f)->next, (zchk(f) ? (zset(l)) : zset((f)->prev))):\
((l)==(n))?\
((l)=(l)->prev, (zchk(l) ? (zset(f)) : zset((l)->next))):\
((zchk((n)->next) ? (0) : ((n)->next->prev=(n)->prev)),\
(zchk((n)->prev) ? (0) : ((n)->prev->next=(n)->next))))

#define QueuePush(f,l,n)         QueuePush_NZ(f,l,n,next,CheckNull,SetNull)
#define QueuePushFront(f,l,n)    QueuePushFront_NZ(f,l,n,next,CheckNull,SetNull)
#define QueuePop(f,l)            QueuePop_NZ(f,l,next,SetNull)
#define StackPush(f,n)           StackPush_N(f,n,next)
#define StackPop(f)              StackPop_NZ(f,next,CheckNull)
#define DLLPushBack(f,l,n)       DLLPushBack_NPZ(f,l,n,next,prev,CheckNull,SetNull)
#define DLLPushFront(f,l,n)      DLLPushBack_NPZ(l,f,n,prev,next,CheckNull,SetNull)
#define DLLInsert(f,l,p,n)       DLLInsert_NPZ(f,l,p,n,next,prev,CheckNull,SetNull)
#define DLLRemove(f,l,n)         DLLRemove_NPZ(f,l,n,next,prev,CheckNull,SetNull)

// Member offset
struct MemberOffset {
 u64 v;
};
#define MemberOff(S, member) (MemberOffset){OffsetOf(S, member)}
#define MemberOffLit(S, member) {OffsetOf(S, member)}
#define MemberFromOff(ptr, type, memoff) (*(type *)((u8 *)ptr + memoff.v))

// String types
// We want to eventually support UTF-8/16/32
// For now, we will mostly work with UTF-8 strings.

struct String8 {
	u8* str;
	u64 size;
};

struct String16 {
	u16* str;
	u64 size;
};

struct String32 {
	u32* str;
	u64 size;
};

}// namespace pm

#include "memory/arena.h"
#include "thread_context.h"
