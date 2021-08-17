#ifndef TKLBZ_MEMORY_MONKEY_PATCH
#define TKLBZ_MEMORY_MONKEY_PATCH

/**
 * Force all other standart allocation operators to use the
 * ones provided by tklb. This is potentially dangerous
 * when linking in libraries and passing memory to them.
 * This rarely ever works in practice but is interesting
 * for educational purposes nevertheless.
 */

#include <cstddef>
#include <new>

#define malloc(size)			TKLB_MALLOC(size)
#define free(ptr)				TKLB_FREE(ptr)
#define realloc(ptr, size)		TKLB_REALLOC(ptr, size)
#define calloc(num, size)		TKLB_CALLOC(num, size)

// Capturing spcific variations of malloc is actual madness
#define _MM_MALLOC_H_INCLUDED
#define __MM_MALLOC_H

// Gets rif of redefinition warnings
#undef _mm_malloc
#undef _mm_free
#define _mm_malloc(size, align)	TKLB_MALLOC_ALIGNED(size, align)
#define _mm_free(ptr)			TKLB_FREE_ALIGNED(ptr)

void* operator new(size_t size) { return TKLB_MALLOC(size); }
void* operator new(size_t size, const std::nothrow_t& tag) noexcept { return TKLB_MALLOC(size); }
void* operator new[](size_t size) { return TKLB_MALLOC(size); }
void* operator new[](size_t size, const std::nothrow_t& tag) noexcept { return TKLB_MALLOC(size); }

void operator delete  (void* ptr) noexcept { TKLB_FREE(ptr); }
void operator delete  (void* ptr, const std::nothrow_t& tag) noexcept { TKLB_FREE(ptr); }
void operator delete[](void* ptr) noexcept { TKLB_FREE(ptr); }
void operator delete[](void* ptr, const std::nothrow_t& tag) noexcept { TKLB_FREE(ptr); }
void operator delete  (void* ptr, std::size_t sz) noexcept { TKLB_FREE(ptr); }
void operator delete[](void* ptr, std::size_t sz) noexcept { TKLB_FREE(ptr); }

#endif // TKLBZ_MEMORY_MONKEY_PATCH
