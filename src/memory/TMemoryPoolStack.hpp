#ifndef _TKLB_MEMORY_POOL_STACK
#define _TKLB_MEMORY_POOL_STACK

#include "./TMemoryPool.hpp"
#include "../util/TAssert.h"

namespace tklb { namespace memory {
	/**
	 * @brief Stack based pool
	 */
	class MemoryPoolStack final : public MemoryPool {
	public:
		MemoryPoolStack(void* pool, Size size) : MemoryPool(pool, size) {
			if (mPool.allocated == 0) {
				// first block marks empty space
				Block& block = *reinterpret_cast<Block*>(mPool.memory);
				block.size = 0;
				block.space = mPool.size;
			}
		}

		void* allocate(Size size) override {
			if (size == 0) { return nullptr; }
			if (size < sizeof(Size)) {
				// min block size since the space will be used when it's free
				// to store the distance to the next block
				size = sizeof(Size);
			}

			size += sizeof(Size); // add space for the size

			// This needs to be enough for the Block struct
			static_assert(sizeof(Block) <= 2 * sizeof(Size), "Not enough space to store spare block!");

			// Make sure the size is aligned
			size += sizeof(uintptr_t) - (size % sizeof(uintptr_t));

			Lock lock(mPool.mutex);
			for (Size i = 0; i < mPool.size;) {
				Block& block = *reinterpret_cast<Block*>(mPool.memory + i);
				if (block.size == 0) {
					// block is free
					if (size <= block.space) {
						// block has space
						const Size oldSize = block.space;
						if (oldSize <= size + sizeof(Size)) {
							// not enough space to mark a spare block
							// after, so just take the space of the old block
							size = oldSize;
						} else {
							// Enough space to mark a new block
							Block& freeBlock = *reinterpret_cast<Block*>(
								reinterpret_cast<Byte*>(&block) + size
							);
							freeBlock.size = 0;
							// Mark the size of the new free block
							freeBlock.space = oldSize - size;
						}
						block.size = size;
						mPool.allocated += size;
						return &block.space; // * Found free spot
					} else {
						// Step over the free area which is too small
						if (block.space == 0) {
							// this means the previous block has overrun this one
							// and we can't continue
							TKLB_ASSERT(false)
							return nullptr; // ! Block was overrun
						}
						i += block.space;
					}
				} else {
					if (block.size == 0) {
						// same as above
						TKLB_ASSERT(false)
						return nullptr; // ! Block was overrun
					}
					i += block.size; // Step over the already allocated area
				}
			}
			TKLB_ASSERT(false)
			return nullptr; // ! No memory left
		}

		void deallocate(void* ptr) override {
			if (ptr == nullptr) { return; }

			// Check if pointer is in memory range
			TKLB_ASSERT((uintptr_t) mPool.memory <= (uintptr_t) ptr)
			TKLB_ASSERT((uintptr_t) ptr < (uintptr_t) mPool.memory + (uintptr_t)mPool.memory)

			Block& block = *reinterpret_cast<Block*>(
				reinterpret_cast<Byte*>(ptr) - sizeof(Size)
			);

			// blocks can never be less than 2 * sizeof(Size)
			TKLB_ASSERT(sizeof(Size) < block.size)

			#ifdef TKLB_MEM_TRACE
				Size* data = reinterpret_cast<Size*>(&block);
				const Size end = block.size /  sizeof(Size);
				// Set the freed memory to a pattern
				// skip the first 8 bytes since they are 0 indicating the block is free
				for (Size i = 2; i < end; i++) {
					data[i] = 123;
				}
				data[end - 1] = 666; // end of free block
			#endif

			Lock lock(mPool.mutex);
			// Save how large is the gap in memory will be
			// TODO tklb check if the next block is free too
			// These blocks should be merged or fragmentation gets worse
			block.space = block.size;
			mPool.allocated -= block.size;
			block.size = 0; // Mark the block as unallocated
		}

		void* reallocate(void* ptr, size_t size) override {
			// Act like malloc when ptr is nullptr
			if (ptr == nullptr) { return allocate(size); }

			Block& block = *reinterpret_cast<Block*>(
				reinterpret_cast<Byte*>(ptr) - sizeof(Size)
			);
			const Size oldSize = block.size;
			Size newSize = size + sizeof(Size);
			// Make sure the size is aligned
			newSize += sizeof(uintptr_t) - (newSize % sizeof(uintptr_t));

			if (newSize <= oldSize) {
				// Down size
				if (oldSize <= newSize + sizeof(Size)) {
					// Don't do anything since there's not enough
					// space beeing freed to mark a spare block
					return ptr; // * return same block
				}

				// Enough space for new spare block
				block.size = newSize; // down size current block
				Block& freeBlock = *reinterpret_cast<Block*>(
					reinterpret_cast<Byte*>(&block) + newSize
				);
				freeBlock.size = 0; // Mark the start of a new block
				freeBlock.space = oldSize - newSize; // Mark the size of the new free block
				return ptr; // * return same block
			}

			void* newPtr = allocate(size);
			if (newPtr == nullptr) { return nullptr; }
			const Size bytes = oldSize - sizeof(Size);
			copy(ptr, newPtr, bytes);
			deallocate(ptr);
			return newPtr;
		};
	};

} } // namespace tklb::memory

#endif // _TKLB_MEMORY_POOL_STACK
