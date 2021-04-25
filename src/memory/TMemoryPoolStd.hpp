#ifndef TKLBZ_MEMORY_POOL_STD
#define TKLBZ_MEMORY_POOL_STD

#include <stdlib.h>

#include "./TMemoryPool.hpp"
#include "../util/TAssert.h"

namespace tklb { namespace memory {
	/**
	 * @brief Stack based pool
	 */
	class MemoryPoolStd final : public MemoryPool {
		// space for a SharedPool struct
		char mDummyPool[sizeof(SharedPool)];
		MemoryPoolStd() : MemoryPool(mDummyPool, sizeof(SharedPool)) { }
		void* (* const mAllocate)(size_t) = malloc;
		void* (* const mReallocate)(void*, size_t) = realloc;
		void  (* const mDeallocate)(void*) = free;

		struct SizeBlock {
			Size size;
			void* ptr;
		};
	public:
		/**
		 * @brief The pool uses the standard allocator
		 * So no need for multiple instances.
		 */
		static MemoryPoolStd& instance() {
			static MemoryPoolStd i;
			return i;
		}

		void* allocate(Size size) override {
			void* ptr = mAllocate(size + sizeof(Size));

			if (ptr == nullptr) { return nullptr; }
			mPool.allocated += size;

			SizeBlock& block = *static_cast<SizeBlock*>(ptr);
			block.size = size;

			return &block.ptr;
		}

		void deallocate(void* ptr) override {
			if (ptr == nullptr) { return; }
			SizeBlock& block = *reinterpret_cast<SizeBlock*>(
				static_cast<char*>(ptr) - sizeof(Size)
			);
			mPool.allocated -= block.size;
			return mDeallocate(&block);
		}

		void* reallocate(void* ptr, size_t size) override {
			if (ptr == nullptr) {
				return allocate(size);
			}
			void* ptrNew = mReallocate(ptr, size + sizeof(Size));

			SizeBlock& blockOld = *reinterpret_cast<SizeBlock*>(
				static_cast<char*>(ptr) - sizeof(Size)
			);
			mPool.allocated -= blockOld.size;
			if (ptrNew == nullptr) { return nullptr; }

			mPool.allocated += size;

			SizeBlock& blockNew = *static_cast<SizeBlock*>(ptr);
			blockNew.size = size;

			return &blockNew.ptr;
		}
	};

} } // namespace tklb::memory

#endif // TKLBZ_MEMORY_POOL_STD
