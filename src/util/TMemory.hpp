#ifndef TKLBZ_MEMORY
#define TKLBZ_MEMORY

#include <stddef.h>
#include <utility>

#ifndef TKLB_MEM_NO_STD
	#include <cstring>
	#include <stdlib.h>
#endif

#ifdef TKLB_MEM_OVERLOAD_ALL
	#include <new>
#endif

#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
	#include "../../external/xsimd/include/xsimd/config/xsimd_config.hpp"
#endif

#include "./TAssert.h"

namespace tklb {
	/**
	 * Wraps allocation so using a custom memory manager
	 * is an option
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


		static inline void* clearallocate(size_t num, size_t size) noexcept {
			const size_t total = size * num;
			void* ptr = allocate(total);
			if (ptr == nullptr) { return nullptr; }
			set(ptr, 0, total);
			return ptr;
		}

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
		static void* allocateAligned(const size_t bytes, const size_t align = DEFAULT_ALIGN) noexcept {
			#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
				// malloc is already already aligned to sizeof(size_t)
				void* result = allocate(bytes + align);
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
				return allocate(bytes);
			#endif // defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
		}

		/**
		 * @brief Allocate and construct object
		 * @param args Arguments passed to class contructor
		 */
		template <class T, typename ... Args>
		static T* create(Args&& ... args) {
			void* ptr = allocate(sizeof(T));
			if (ptr != nullptr) {
				T* test = new (ptr) T(std::forward<Args>(args)...);
			}
			return reinterpret_cast<T*>(ptr);
		}

		/**
		 * @brief Destroy the object and dispose the memory
		 */
		template <class T>
		static void dispose(T* ptr) {
			if (ptr != nullptr) {
				ptr->~T();
				deallocate(ptr);
			}
		}

		/**
		 * Tracer functions also taking file and line as arguments
		 */
	#ifdef TKLB_MEM_TRACE
		static inline void* allocateTrace(size_t size, const char* file, int line) noexcept {
			return allocate(size);
		};

		static inline void* reallocateTrace(void* ptr, size_t size, const char* file, int line) noexcept {
			return reallocate(ptr, size);
		};

		static inline void* clearallocateTrace(size_t num, size_t size, const char* file, int line) noexcept {
			return clearallocate(num, size);
		};

		static inline void deallocateTrace(void* ptr, const char* file, int line) noexcept {
			deallocate(ptr);
		};

		static inline void deallocateAlignedTrace(void* ptr, const char* file, int line) noexcept {
			deallocateAligned(ptr);
		};

		static inline void* allocateAlignedTrace(const char* file, int line, size_t bytes, size_t align = DEFAULT_ALIGN) noexcept {
			return allocateAligned(bytes, align);
		};

		template <class T, typename ... Args>
		static inline T* createTrace(const char* file, int line, Args&& ... args) {
			return create<T>(std::forward<Args>(args)...);
		}

		template <class T>
		static inline void disposeTrace(T* ptr, const char* file, int line) {
			dispose(ptr);
		}
	#endif // TKLB_MEM_TRACE
	}
}

/**
 *
 * Wrapped in macros so information about where the allocations happened can be gathered
 *
 */
#ifdef TKLB_MEM_TRACE
	#define TKLB_MALLOC(size)				tklb::memory::allocateTrace(size, __FILE__, __LINE__)
	#define TKLB_FREE(ptr)					tklb::memory::deallocateTrace(ptr, __FILE__, __LINE__)
	#define TKLB_REALLOC(ptr, size) 		tklb::memory::reallocateTrace(ptr, size, __FILE__, __LINE__)
	#define TKLB_CALLOC(num, size) 			tklb::memory::clearallocateTrace(num, size, __FILE__, __LINE__)
	#define TKLB_MALLOC_ALIGNED(size, ...)	tklb::memory::allocateAlignedTrace(__FILE__, __LINE__, size, ##__VA_ARGS__)
	#define TKLB_FREE_ALIGNED(ptr)			tklb::memory::deallocateAlignedTrace(ptr, __FILE__, __LINE__)
	#define TKLB_NEW(T, ...)				tklb::memory::createTrace<T>(__FILE__, __LINE__, ##__VA_ARGS__)
	#define TKLB_DELETE(ptr)				tklb::memory::disposeTrace(ptr, __FILE__, __LINE__)
#else // TKLB_MEM_TRACE
	#define TKLB_MALLOC(size)				tklb::memory::allocate(size)
	#define TKLB_FREE(ptr)					tklb::memory::deallocate(ptr)
	#define TKLB_REALLOC(ptr, size)			tklb::memory::reallocate(ptr, size)
	#define TKLB_CALLOC(num, size) 			tklb::memory::clearallocate(num, size)
	#define TKLB_MALLOC_ALIGNED(size, ...)	tklb::memory::allocateAligned(size, ##__VA_ARGS__)
	#define TKLB_FREE_ALIGNED(ptr)			tklb::memory::deallocateAligned(ptr)
	#define TKLB_NEW(T, ...)				tklb::memory::create<T>(__VA_ARGS__)
	#define TKLB_DELETE(ptr)				tklb::memory::dispose(ptr)
#endif // TKLB_MEM_TRACE


/**
 *
 * Force all other standart allocation operators to use the
 * ones provided by tklb. This is potentially dangerous
 * when linking in libraries and passing memory to them.
 * This rarely ever works in practice but is interesting
 * for educational purposes nevertheless.
 *
 */
#ifdef TKLB_MEM_OVERLOAD_ALL
	#define malloc(size)			TKLB_MALLOC(size)
	#define free(ptr)				TKLB_FREE(ptr)
	#define realloc(ptr, size)		TKLB_REALLOC(ptr, size)
	#define calloc(num, size)		TKLB_CALLOC(num, size)

	// Capturing spcific variations of malloc is actual madness
	#define _MM_MALLOC_H_INCLUDED
	#define __MM_MALLOC_H

	// Gets rif of compiler redefinition warnings
	#undef _mm_malloc
	#undef _mm_free
	#define _mm_malloc(size, align)	TKLB_MALLOC_ALIGNED(size, align)
	#define _mm_free(ptr)			TKLB_FREE_ALIGNED(ptr)

	void* operator new(size_t size) { return TKLB_MALLOC(size); }
	void* operator new(size_t size, const std::nothrow_t& tag) noexcept { return TKLB_MALLOC(size); }
	void* operator new[](size_t size) { return TKLB_MALLOC(size); }
	void* operator new[](size_t size, const std::nothrow_t& tag) noexcept { return TKLB_MALLOC(size); }

	void operator delete  (void* ptr) noexcept { TKLB_FREE(ptr); }
	void operator delete  (void* ptr, const std::nothrow_t& tag) noexcept { TKLB_FREE(ptr); }
	void operator delete[](void* ptr) noexcept { TKLB_FREE(ptr); }
	void operator delete[](void* ptr, const std::nothrow_t& tag) noexcept { TKLB_FREE(ptr); }
	void operator delete  (void* ptr, std::size_t sz) noexcept { TKLB_FREE(ptr); }
	void operator delete[](void* ptr, std::size_t sz) noexcept { TKLB_FREE(ptr); }
#endif // TKLB_MEM_OVERLOAD_ALL

#endif // TKLB_MEMORY
