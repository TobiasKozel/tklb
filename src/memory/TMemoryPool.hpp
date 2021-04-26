#ifndef TKLBZ_MEMORY_POOL
#define TKLBZ_MEMORY_POOL

#include <cstdint> // For uintptr_t
#include <new> // for placement new with parameters

#include "./TMemoryUtil.hpp"
#include "../util/TAssert.h"
#include "../types/TSpinLock.hpp"
#include "../types/TLockGuard.hpp"

#ifndef TKLB_NO_SIMD
	#include "../../external/xsimd/include/xsimd/config/xsimd_config.hpp"
#endif

namespace tklb { namespace memory {

	/**
	 * @brief Memory pool interface.
	 */
	class MemoryPool {
	protected:
		// Type used to store sizes,
		// seems wasteful on 64bit machines but memory needs to be aligned
		// so it behaves like gcc malloc
		using Size = uintptr_t;
		using Byte = unsigned char;
		using Mutex = SpinLock;
		using Lock = LockGuard<Mutex>;

		#ifndef TKLB_NO_SIMD
			static constexpr Size DEFAULT_ALIGN = XSIMD_DEFAULT_ALIGNMENT;
		#else
			static constexpr Size DEFAULT_ALIGN = 16;
		#endif

		/**
		 * @brief Struct used to mark every allocation;
		 */
		struct Block {
			// Size stored before actual memory
			Size size;
			// Either the size of free space if size is 0
			// or the start of the memory
			Size space;
		};

		/**
		 * @brief Pools can share the same space, but use it differently.
		 * Info is stored at the beginning of the pool.
		 */
		struct SharedPool {
			Size size = 0; // Total pool size excluding this struct
			Size users = 0; // Keeps track of how many pools share the memory
			Mutex mutex; // Locking isn't ideal but easy
			// Used space
			Size allocated = 0;
			// Start of the usable pool space
			// gdb watch expression for convenience
			// *(tklb::memory::manager::Size(*)[200])tklb::memory::manager::CustomMemory
			Byte* memory = nullptr;
		};

		SharedPool& mPool;

	public:
		/**
		 * @brief Construct a pool from memory provided.
		 * The first few bytes need to be zero to indicate
		 * when the pool needs to be initialized.
		 */
		MemoryPool(void* pool, Size size) : mPool(*static_cast<SharedPool*>(pool)) {
			if (mPool.size == 0) { // New pool
				TKLB_ASSERT(sizeof(SharedPool) <= size)
				// Initialize pool
				SharedPool* p = new (pool) SharedPool();
				mPool.size = size - sizeof(SharedPool);
				mPool.memory = reinterpret_cast<Byte*>(p + 1);
			}
			TKLB_ASSERT(size == (mPool.size + sizeof(SharedPool)))
			mPool.users++;
		}

		~MemoryPool() {
			TKLB_ASSERT(mPool.users != 0)
			mPool.users--;
			if (mPool.users == 0) {
				TKLB_ASSERT(mPool.allocated == 0)
			}
		}

		virtual void* allocate(Size size) = 0;

		virtual void deallocate(void* ptr) = 0;

		virtual void* reallocate(void* ptr, size_t size) = 0;

		/**
		 * @brief Allocates space for num of structs with size size and clears the memory
		 * @param num Number of structs
		 * @param size Size of a single struct
		 */
		void* clearallocate(size_t num, size_t size) {
			const size_t total = size * num;
			void* ptr = allocate(total);
			if (ptr == nullptr) { return nullptr; }
			set(ptr, 0, total);
			return ptr;
		}

		/**
		 * @brief Acts like new. Allocate and construct object.
		 * @param args Arguments passed to class contructor
		 */
		template <class T, typename ... Args>
		T* create(Args&& ... args) {
			void* ptr = allocate(sizeof(T));
			if (ptr == nullptr) { return nullptr; }
			new (ptr) T(std::forward<Args>(args)...);
			return reinterpret_cast<T*>(ptr);
		}

		/**
		 * @brief Acts like delete. Destroy the object and dispose the memory.
		 */
		template <class T>
		void dispose(T* ptr) {
			if (ptr == nullptr) { return; }
			ptr->~T();
			deallocate(ptr);
		}

		void deallocateAligned(void* ptr) {
			if (ptr == nullptr) { return; }
			// Get the orignal allocation address to free the memory
			deallocate(*(reinterpret_cast<void**>(ptr) - 1));
		}

		/**
		 * @brief Allocate aligned if simd is enabled.
		 * Does a normal allocation otherwise.
		 */
		void* allocateAligned(const Size size, const Size align = DEFAULT_ALIGN) {
			// malloc is already already aligned to sizeof(size_t)
			void* result = allocate(size + align);
			if (result != nullptr && align != 0) {
				// Mask with zeroes at the end to floor the pointer to an aligned block
				const Size mask = ~(Size(align - 1));
				const Size pointer = reinterpret_cast<Size>(result);
				const Size floored = pointer & mask;
				const Size aligned = floored + align;

				// Not enough space before aligned memory to store original ptr
				// This only happens when malloc doesn't align to sizeof(size_t)
				TKLB_ASSERT(sizeof(Size) <= (aligned - pointer))

				result = reinterpret_cast<void*>(aligned);
				Size* original = reinterpret_cast<Size*>(result) - 1;
				*(original) = pointer;
			}
			TKLB_ASSERT(Size(result) % align == 0)
			return result;
		}
	};
} } // namespace tklb::memory

#endif // TKLBZ_MEMORY_POOL
