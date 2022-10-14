#ifndef _TKLB_MEMORY_POOL_STACK
#define _TKLB_MEMORY_POOL_STACK

#include <cstdint>	// uintptr_t

#include "../types/TSpinLock.hpp"
#include "../types/TLockGuard.hpp"
#include "./TMemory.hpp"
#include "../util/TMath.hpp"

#ifndef TKLB_ASSERT
	#define TKLB_ASSERT(cond)
#endif

namespace tklb { namespace memory {

	/**
	 * @brief This class can manage sub allocations of arbitrary size from a
	 *        chunk of memory provided beforehand.
	 */
	class FixedPool {
		/**
		 * @brief Type used to store sizes,
		 * seems wasteful on 64bit machines but memory needs to be aligned
		 * so it behaves like gcc malloc
		 */
		using Size = uintptr_t;
		using Byte = unsigned char;
		using Mutex = SpinLock;
		using Lock = LockGuard<Mutex>;

		/**
		 * @brief Struct used to mark every allocation;
		 */
		struct Block {
			/**
			 * @brief The size of the allocation following this block
			 */
			Size size;
			/**
			 * @brief The usable memory of a block starts AT this variable.
			 *        If size is 0, this tells us how much free space there is
			 *        before the next block or end of the pool.
			 */
			Size space;
		};

		// This needs to be enough for the Block struct
		static_assert(sizeof(Block) <= 2 * sizeof(Size), "Not enough space to store spare block!");

		Size poolSize = 0;			///< Total pool size excluding this struct
		Mutex mutex;				///< Locking isn't ideal but easy
		Size allocatedSpace = 0;	///< Used space
		/**
		 * Start of the usable pool space
		 * this does not store the pointer, but the location location itself is used
		 */
		Byte* memory = nullptr;

	public:
		FixedPool(void* pool, Size size) {
			memory = reinterpret_cast<Byte*>(pool);

			// first block marks empty space
			Block& block = *reinterpret_cast<Block*>(memory);
			block.size = 0;
			block.space = size;
			poolSize = size;
		}

		~FixedPool() {
			TKLB_ASSERT(allocatedSpace == 0)
		}

		/**
		 * @brief How many bytes are already allocated
		 * @return Size
		 */
		Size allocated() const { return allocatedSpace; }

		/**
		 * @brief How many more bytes are theoretically left.
		 *        However an allocation of a large block can still fail, if no
		 *        sequential space large enough is free!
		 * @return Size
		 */
		Size free() const { return poolSize - allocatedSpace; }

		/**
		 * @brief Since there's also meta data about each allocation,
		 *        more space is needed to do an allocation.
		 * @param size The requested space
		 * @return constexpr Size How much space the allocation will actually take
		 */
		static constexpr Size realAllocation(Size size) {
			return size == 0 ? 0 :
				// once the block is freed, we need at least that much space
				// to indicate where the next memory block starts.
				max((Size) sizeof(Size), size) +
				// TODO this seems broken
				// then, while the block is allocated, we need to store the
				// allocation size before the actual memory starts
				sizeof(Size);
				// sizeof(uintptr_t) - (size % sizeof(uintptr_t));
		}

		void* allocate(Size size) {
			if (size == 0) { return nullptr; }

			size = realAllocation(size);

			Lock lock(mutex);
			for (Size i = 0; i < poolSize;) {
				Block& block = *reinterpret_cast<Block*>(memory + i);
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
						allocatedSpace += size;
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

		void deallocate(void* ptr) {
			if (ptr == nullptr) { return; }

			// Check if pointer is in memory range
			TKLB_ASSERT((uintptr_t) memory <= (uintptr_t) ptr)
			TKLB_ASSERT((uintptr_t) ptr < (uintptr_t) memory + (uintptr_t) memory)

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

			Lock lock(mutex);
			// Save how large is the gap in memory will be
			// TODO tklb check if the next block is free too
			// These blocks should be merged or fragmentation gets worse
			block.space = block.size;
			allocatedSpace -= block.size;
			block.size = 0; // Mark the block as unallocated
		}

		void* reallocate(void* ptr, size_t size) {
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
