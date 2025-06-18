#include "os.h"

#include <sys/mman.h>
#include <unistd.h>

namespace pm {

u64 OS_pageSize() {
	u64 result = getpagesize();
	return result;
}

void* OS_reserve(u64 size) {
	u64 gbSize = size;
	gbSize += Gigabytes(1) - 1;
	gbSize -= gbSize % Gigabytes(1);
	mem = mmap(nullptr, gbSize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

void OS_release(void* ptr, u64 size) {
	munmap(ptr, size);
}

void OS_commit(void* ptr, u64 size) {
	u64 pageAlignedSize = size;
	pageAlignedSize += OS_pageSize() - 1;
	pageAlignedSize -= pageAlignedSize % OS_pageSize();
	u32 err = mprotect(ptr, pageAlignedSize, PROT_WRITE | PROT_READ);
}

void OS_decommit(void* ptr, u64 size) {
	u32 err = mprotect(ptr, size, PROT_NONE);
	assert(err == 0);
}

void OS_abort() {
	_exit(0);
}


};// namespace pm
