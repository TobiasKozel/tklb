#ifndef TKLBZ_MEMORY
#define TKLBZ_MEMORY

#include "./TMemoryPool.hpp"
#include "./TMemoryPoolStd.hpp"
#include "./TMemoryPoolStack.hpp"
namespace tklb { namespace memory {
	/**
	 * Defaut pool used when nothing else is specified
	 * Will be define in every compilation unit, but the referenced object is the same
	 */
	static MemoryPool& DefaultPool = MemoryPoolStd::instance();
	// constexpr int PoolSize = 300 * 1024 * 1024;
	// MemoryPoolStack DefaultPool = MemoryPoolStack(malloc(PoolSize), PoolSize);
} } // namespace tklb::memory


// Macros for easy default Pool access
#define TKLB_MALLOC(size)				tklb::memory::DefaultPool.allocate(size)
#define TKLB_FREE(ptr)					tklb::memory::DefaultPool.deallocate(ptr)
#define TKLB_REALLOC(ptr, size)			tklb::memory::DefaultPool.reallocate(ptr, size)
#define TKLB_CALLOC(num, size) 			tklb::memory::DefaultPool.clearallocate(num, size)
#define TKLB_MALLOC_ALIGNED(size, ...)	tklb::memory::DefaultPool.allocateAligned(size, ##__VA_ARGS__)
#define TKLB_FREE_ALIGNED(ptr)			tklb::memory::DefaultPool.deallocateAligned(ptr)
#define TKLB_NEW(T, ...)				tklb::memory::DefaultPool.create<T>(__VA_ARGS__)
#define TKLB_DELETE(ptr)				tklb::memory::DefaultPool.dispose(ptr)
#define TKLB_CHECK_HEAP()

#ifdef TKLB_MEM_MONKEY_PATCH
	#include "./TMemoryMonkeyPatch.hpp"
#endif

#endif // TKLB_MEMORY
