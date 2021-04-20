#ifndef TKLBZ_MEMORY_MANAGER
#define TKLBZ_MEMORY_MANAGER

#include "./../util/TAssert.h"
#include "./../util/TMath.hpp"

#include "./TMemory.hpp"
#include "./TAllocator.hpp"

// For uintptr_t
#include <cstdint>

namespace tklb {
	namespace memory {
		/**
		 * @brief Extremely basic optional Memorymanager which works with a preallocated
		 * chunk of memory. Fragmentation is probably quite an issue.
		 * From a security standpoint this is horrible since the whole memorylayout
		 * is part of the memory.
		 * [size|data],[size|data],[0|size of free block]
		 * example:
		 * size is stored in a 4 byte int and includes itself
		 * [12|Space of 8 bytes],
		 * [8|space of 4 bytes],
		 * [0,2](4 byte of actual free space),
		 * [0,0](free space till end of buffer)
		 */
		namespace manager {
			// Type used to store sizes,
			// seems wasteful on 64bit machines but memory needs to be aligned
			// so it behaves like gcc malloc
			using Size = uintptr_t;
			using Byte = unsigned char;

			/**
			 * @brief Struct making the access of a memory block easier
			 */
			struct Block {
				// Size stored before actual memory
				Size size;
				// Either the size of free space if size is 0
				// or the start of the memory
				Size space;
			};

			Size CustomSize = 0;
			// gdb watch expression for convenience
			// *(tklb::memory::manager::Size(*)[200])tklb::memory::manager::CustomMemory
			Byte* CustomMemory = nullptr;

			/**
			 * @brief Looks for next free area with enough space
			 */
			void* allocate(size_t size) {
				if (size == 0) { return nullptr; }
				if (size < sizeof(Size)) {
					// min block size since the space will be used when it's free
					// to store the distance to the next block
					size = sizeof(Size);
				}

				// add space for the size
				size += sizeof(Size);

				// This needs to be enough for the Block struct
				static_assert(sizeof(Block) <= 2 * sizeof(Size), "Not enough space to store spare block!");

				// Make sure the size is aligned
				size += sizeof(uintptr_t) - (size % sizeof(uintptr_t));
				for (Size i = 0; i < CustomSize;) {
					Block& block = *reinterpret_cast<Block*>(CustomMemory + i);
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
							Allocated += size;
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
				TKLB_ASSERT((uintptr_t) CustomMemory <= (uintptr_t) ptr)
				TKLB_ASSERT((uintptr_t) ptr < (uintptr_t) CustomMemory + (uintptr_t)CustomSize)

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

				// Save how large is the gap in memory will be
				// TODO tklb check if the next block is free too
				// These blocks should be merged or fragmentation gets worse
				block.space = block.size;
				Allocated -= block.size;
				block.size = 0; // Mark the block as unallocated
			}

			void* reallocate(void* ptr, size_t size) {
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
			}

			/**
			 * @brief Use the manager with a preallocated block.
			 */
			void use(void* memory, size_t size) {
				CustomSize = size;
				CustomMemory = static_cast<Byte*>(memory);

				// Replace the allocation functions with the managers
				tklb::memory::allocate = allocate;
				tklb::memory::reallocate = reallocate;
				tklb::memory::deallocate = deallocate;

				#ifdef TKLB_MEM_TRACE
					// When tracing memory the space will be filled with numbers
					// enumerating each address
					for (Size i = 0; i < CustomSize / sizeof(Size); i++) {
						reinterpret_cast<Size*>(CustomMemory)[i] = i;
					}
					// set(CustomMemory, 0, CustomSize);
				#endif

				// Mark the whole area as a free block
				Block& block = *reinterpret_cast<Block*>(CustomMemory);
				block.size = 0;
				block.space = CustomSize;

				#ifdef TKLB_MEM_TRACE
					tklb::memory::tracer::init();
				#endif
			}

			/**
			 * @brief Restore the standart allocatio functions if there are any.
			 * Prevents crashes when objects constructed by the crt allocate
			 * and try to free it again on programm termination.
			 */
			void restore() {
				#ifdef TKLB_MEM_TRACE
					tklb::memory::tracer::stop();
				#endif
				tklb::memory::allocate = std_allocate;
				tklb::memory::reallocate = std_reallocate;
				tklb::memory::deallocate = std_deallocate;
			}
		} // namespace manager
	} // namespace memory
} // namespace tklb

#endif // TKLBZ_MEMORY_MANAGER
