#ifndef _TKLB_MEMORY_MONKEY_PATCH
#define _TKLB_MEMORY_MONKEY_PATCH

/**
 * @file TMemoryMonkeyPatch.hpp
 * @author Tobias Kozel
 * @brief Attempt at patching all memory allocations
 * @details
 * ! TODO unfinished mess
 * Force all other standart allocation operators to use the
 * ones provided by tklb. This is potentially dangerous
 * when linking in libraries and passing memory to them.
 * This rarely ever works in practice but is interesting
 * for educational purposes nevertheless.
 * @version 0.1
 * @date 2022-08-05
 *
 * @copyright Copyright (c) 2022
 *
 */

 #include <new>			// For placement new with parameters
 #include <stddef.h>	// size_t

#define malloc(size)			TKLB_MALLOC(size)
#define free(ptr)				TKLB_FREE(ptr)
// #define realloc(ptr, size)		TKLB_REALLOC(ptr, size)
// #define calloc(num, size)		TKLB_CALLOC(num, size)

// // Capturing specific variations of malloc is actual madness
// #define _MM_MALLOC_H_INCLUDED
// #define __MM_MALLOC_H

// // Gets rif of redefinition warnings
// #undef _mm_malloc
// #undef _mm_free
// #define _mm_malloc(size, align)	TKLB_MALLOC_ALIGNED(size, align)
// #define _mm_free(ptr)			TKLB_FREE_ALIGNED(ptr)

#ifdef TKLB_IMPL
	#define _TKLB_HEADER_DEFINTION(body) { body }
#else
	#define _TKLB_HEADER_DEFINTION(body) ;
#endif

void* operator new(size_t size)
	_TKLB_HEADER_DEFINTION(return TKLB_MALLOC(size);)
void* operator new(size_t size, const std::nothrow_t& tag) noexcept
	_TKLB_HEADER_DEFINTION(return TKLB_MALLOC(size);)
void* operator new[](size_t size)
	_TKLB_HEADER_DEFINTION(return TKLB_MALLOC(size);)
void* operator new[](size_t size, const std::nothrow_t& tag) noexcept
	_TKLB_HEADER_DEFINTION(return TKLB_MALLOC(size);)

void operator delete  (void* ptr) noexcept
	_TKLB_HEADER_DEFINTION(TKLB_FREE(ptr);)
void operator delete  (void* ptr, const std::nothrow_t& tag) noexcept
	_TKLB_HEADER_DEFINTION(TKLB_FREE(ptr);)
void operator delete[](void* ptr) noexcept
	_TKLB_HEADER_DEFINTION(TKLB_FREE(ptr);)
void operator delete[](void* ptr, const std::nothrow_t& tag) noexcept
	_TKLB_HEADER_DEFINTION(TKLB_FREE(ptr);)
void operator delete  (void* ptr, std::size_t sz) noexcept
	_TKLB_HEADER_DEFINTION(TKLB_FREE(ptr);)
void operator delete[](void* ptr, std::size_t sz) noexcept
	_TKLB_HEADER_DEFINTION(TKLB_FREE(ptr);)

#endif // _TKLB_MEMORY_MONKEY_PATCH
