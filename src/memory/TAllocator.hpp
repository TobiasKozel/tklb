#ifndef TKLBZ_ALLOCATOR
#define TKLBZ_ALLOCATOR


#include <stddef.h> // needed for size_t
#include <utility>

#ifndef TKLB_MEM_NO_STD
	#include <cstring>
	#include <stdlib.h>
#endif

#include <new> // needed for placement new with parameters

#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
	#include "../../external/xsimd/include/xsimd/config/xsimd_config.hpp"
#endif

#include "./../util/TAssert.h"

namespace tklb {
	/**
	 * Wraps allocation so using a custom memory manager
	 * is an option. Not a std C++ allocator
	 */
	namespace memory {
		// This can be used to keep track of allocations
		// However only the custom memorymanager keeps track for now
		// stdlib or external allocation functions would need to be wrapped
		inline size_t Allocated = 0;

		#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
			constexpr unsigned int DEFAULT_ALIGN = XSIMD_DEFAULT_ALIGNMENT;
		#else
			constexpr unsigned int DEFAULT_ALIGN = sizeof(size_t);
		#endif

	#ifdef TKLB_MEM_NO_STD
		inline void* (*allocate)(size_t) = nullptr;
		inline void* (*reallocate)(void*, size_t) = nullptr;
		inline void (*deallocate)(void*) = nullptr;

		// They will stay empty
		inline void* (*std_allocate)(size_t) = nullptr;
		inline void* (*std_reallocate)(void*, size_t) = nullptr;
		inline void (*std_deallocate)(void*) = nullptr;
	#else
		inline void* (*allocate)(size_t) = malloc;
		inline void* (*reallocate)(void*, size_t) = realloc;
		inline void (*deallocate)(void*) = free;

		// Store these away in case they are needed
		// when interacting with linked libs
		// the ones above might be replaced with a custom one
		inline void* (*std_allocate)(size_t) = malloc;
		inline void* (*std_reallocate)(void*, size_t) = realloc;
		inline void (*std_deallocate)(void*) = free;
	#endif

		/**
		 * @brief memcpy wrapper
		 */
		static inline void copy(void* dst, const void* src, const size_t size) {
		#ifdef TKLB_MEM_NO_STD
			auto source = reinterpret_cast<const unsigned char*>(src);
			auto destination = reinterpret_cast<unsigned char*>(dst);
			for (size_t i = 0; i < size; i++) {
				destination[i] = source[i];
			}
		#else // TKLB_MEM_NO_STD
			memcpy(dst, src, size);
		#endif // TKLB_MEM_NO_STD
		}

		/**
		 * @brief memset wrapper
		 */
		static inline void set(void* dst, const unsigned char val, size_t size) {
		#ifdef TKLB_MEM_NO_STD
			auto pointer = reinterpret_cast<unsigned char*>(dst);
			for (size_t i = 0; i < size; i++) {
				pointer[i] = val;
			}
		#else // TKLB_MEM_NO_STD
			memset(dst, val, size);
		#endif // TKLB_MEM_NO_STD
		}

		/**
		 * @brief Allocates space for num of structs with size size and clears the memory
		 * @param num Number of structs
		 * @param size Size of a single struct
		 */
		static inline void* clearallocate(size_t num, size_t size) noexcept {
			const size_t total = size * num;
			void* ptr = allocate(total);
			if (ptr == nullptr) { return nullptr; }
			set(ptr, 0, total);
			return ptr;
		}

		/**
		 * @brief Aligned memory needs to be freed with this function.
		 */
		static inline void deallocateAligned(void* ptr) noexcept {
			#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
				if (ptr == nullptr) { return; }
				// Get the orignal allocation address to free the memory
				deallocate(*(reinterpret_cast<void**>(ptr) - 1));
			#else
				deallocate(ptr);
			#endif
		}

		/**
		 * @brief Allocate aligned if simd is enabled.
		 * Does a normal allocation otherwise.
		 */
		static void* allocateAligned(const size_t size, const size_t align = DEFAULT_ALIGN) noexcept {
			#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
				// malloc is already already aligned to sizeof(size_t)
				void* result = allocate(size + align);
				if (result != nullptr && align != 0) {
					// Mask with zeroes at the end to floor the pointer to an aligned block
					const size_t mask = ~(size_t(XSIMD_DEFAULT_ALIGNMENT - 1));
					const size_t pointer = reinterpret_cast<size_t>(result);
					const size_t floored = pointer & mask;
					const size_t aligned = floored + align;

					// Not enough space before aligned memory to store original ptr
					// This only happens when malloc doesn't align to sizeof(size_t)
					TKLB_ASSERT(sizeof(size_t) <= (aligned - pointer))

					result = reinterpret_cast<void*>(aligned);
					size_t* original = reinterpret_cast<size_t*>(result) - 1;
					*(original) = pointer;
				}
				return result;
			#else
				return allocate(size);
			#endif // defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
		}

		/**
		 * @brief Allocate and construct object
		 * @param args Arguments passed to class contructor
		 */
		template <class T, typename ... Args>
		static T* create(Args&& ... args) {
			void* ptr = allocate(sizeof(T));
			if (ptr == nullptr) { return nullptr; }
			new (ptr) T(std::forward<Args>(args)...);
			return reinterpret_cast<T*>(ptr);
		}

		/**
		 * @brief Destroy the object and dispose the memory
		 */
		template <class T>
		static void dispose(T* ptr) {
			if (ptr == nullptr) { return; }
			ptr->~T();
			deallocate(ptr);
		}


	} // namespace memory
} // namespace tklb

#endif // TKLBZ_ALLOCATOR
