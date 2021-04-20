#ifndef TKLBZ_MEMORY
#define TKLBZ_MEMORY

#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
	#include "../../external/xsimd/include/xsimd/config/xsimd_config.hpp"
#endif

#ifndef TKLB_MEM_NO_STD
	#include <stdlib.h>
#endif

#include "./TAllocator.hpp"

#ifndef TKLB_MEM_TRACE
	#define TKLB_MALLOC(size)				tklb::memory::allocate(size)
	#define TKLB_FREE(ptr)					tklb::memory::deallocate(ptr)
	#define TKLB_REALLOC(ptr, size)			tklb::memory::reallocate(ptr, size)
	#define TKLB_CALLOC(num, size) 			tklb::memory::clearallocate(num, size)
	#define TKLB_MALLOC_ALIGNED(size, ...)	tklb::memory::allocateAligned(size, ##__VA_ARGS__)
	#define TKLB_FREE_ALIGNED(ptr)			tklb::memory::deallocateAligned(ptr)
	#define TKLB_NEW(T, ...)				tklb::memory::create<T>(__VA_ARGS__)
	#define TKLB_DELETE(ptr)				tklb::memory::dispose(ptr)
	#define TKLB_CHECK_HEAP()
#else
	#include "./TMemoryTracing.hpp" // the same as above will be defined there
#endif

#ifdef TKLB_MEM_OVERLOAD_ALL
	#include "TMemoryOverload.hpp"
#endif // TKLB_MEM_OVERLOAD_ALL

#endif // TKLB_MEMORY
