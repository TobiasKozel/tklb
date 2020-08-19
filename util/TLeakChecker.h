#ifndef TKLB_LEAKCHECKER
#define TKLB_LEAKCHECKER

#include <stddef.h>
#include <stdlib.h>
#include <cassert>
#include <cstring>

#ifdef malloc
	#undef malloc
	#undef free
	#undef realloc
#endif

#ifndef TKLB_MAGIC_NUMBER
	#define TKLB_MAGIC_NUMBER 123456789
#endif

namespace tklb {

static size_t allocationCount = 0;

struct MallocInfo {
	const char* file;
	int line;
	size_t size;
	size_t allocation;
};

void* tklbMalloc(const size_t size, const char* file, int line) {
	if (size == 0) { return nullptr; } // We add bytes, so this is needed
	// Make space for the info struct and a magic number at the end
	MallocInfo* ptr = reinterpret_cast<MallocInfo*>(
		malloc(size + sizeof(MallocInfo) + sizeof(int))
	);
	if (!ptr) { return nullptr; }
	allocationCount++;
	ptr->file = file;
	ptr->line = line;
	ptr->size = size;
	ptr->allocation = allocationCount;

	int* magicNumber = reinterpret_cast<int*>(
		reinterpret_cast<unsigned char*>(ptr) + size
	);
	(*magicNumber) = TKLB_MAGIC_NUMBER;

	return ptr + 1;
}

void tklbFree(void* ptr, const char* file, int line) {
	if (ptr == nullptr) { return; }
	MallocInfo* info = reinterpret_cast<MallocInfo*>(ptr) - 1;

	int* magicNumber = reinterpret_cast<int*>(
		reinterpret_cast<unsigned char*>(info) + info->size
	);
	if ((*magicNumber) != TKLB_MAGIC_NUMBER) {
		// Magic number was overwritten
		assert(false);
	}
	free(info);
	allocationCount--;
}

void* tklbRealloc(void* ptr, size_t size, const char* file, int line) {
	MallocInfo* info = reinterpret_cast<MallocInfo*>(ptr);
	if (ptr == nullptr) { return ptr; }
	void* ptr2 = tklbMalloc(size, file, line);
	if (ptr2 == nullptr) { return ptr; }
	memcpy(ptr2, ptr, info->size < size ? info->size : size);
	tklbFree(ptr, file, line);
	return ptr2;
}

}

/**
 * http://stevehanov.ca/blog/?id=10
 * 
 */
void* operator new(size_t size, const char* file, int line) {
	return tklb::tklbMalloc(size, file, line);
}

void* operator new[](size_t size, const char* file, int line) {
	return tklb::tklbMalloc(size, file, line);
}

void operator delete(void* ptr, size_t size) {
	tklb::tklbFree(ptr, nullptr, 0);
}

void operator delete(void* ptr) noexcept {
	tklb::tklbFree(ptr, nullptr, 0);
}

void operator delete[](void* ptr) noexcept {
	tklb::tklbFree(ptr, nullptr, 0);
}

#define new new(__FILE__, __LINE__)

#define malloc(size)      tklb::tklbMalloc (     size, __FILE__, __LINE__)
#define free(ptr)         tklb::tklbFree   (ptr,       __FILE__, __LINE__)
#define realloc(ptr,size) tklb::tklbRealloc(ptr, size, __FILE__, __LINE__)

#endif // TKLB_LEAKCHECKER
