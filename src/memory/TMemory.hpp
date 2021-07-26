#ifndef TKLBZ_MEMORY
#define TKLBZ_MEMORY

#include "./TMemoryPool.hpp"
#include "./TMemoryPoolStd.hpp"
#include "./TMemoryPoolStack.hpp"

/**
 * Defaut pool used when nothing else is specified
 * Will be define in every compilation unit, but the referenced object is the same
 */
#define TKLB_DEFAULT_POOL tklb::memory::MemoryPoolStd::instance()

#ifndef _DEGUG
namespace tklb { namespace memory {
	/**
	 * Only for debugging!
	 */
	static MemoryPool& DefaultPoolDebug = MemoryPoolStd::instance();
	// constexpr int PoolSize = 300 * 1024 * 1024;
	// MemoryPoolStack DefaultPool = MemoryPoolStack(malloc(PoolSize), PoolSize);
} } // namespace tklb::memory

#endif

// Macros for easy default Pool access
#define TKLB_MALLOC(size)				TKLB_DEFAULT_POOL.allocate(size)
#define TKLB_FREE(ptr)					TKLB_DEFAULT_POOL.deallocate(ptr)
#define TKLB_REALLOC(ptr, size)			TKLB_DEFAULT_POOL.reallocate(ptr, size)
#define TKLB_CALLOC(num, size) 			TKLB_DEFAULT_POOL.clearallocate(num, size)
#define TKLB_MALLOC_ALIGNED(size, ...)	TKLB_DEFAULT_POOL.allocateAligned(size, ##__VA_ARGS__)
#define TKLB_FREE_ALIGNED(ptr)			TKLB_DEFAULT_POOL.deallocateAligned(ptr)
#define TKLB_NEW(T, ...)				TKLB_DEFAULT_POOL.create<T>(__VA_ARGS__)
#define TKLB_DELETE(ptr)				TKLB_DEFAULT_POOL.dispose(ptr)
#define TKLB_CHECK_HEAP()

#ifdef TKLB_MEM_MONKEY_PATCH
	#include "./TMemoryMonkeyPatch.hpp"
#endif

#endif // TKLB_MEMORY
