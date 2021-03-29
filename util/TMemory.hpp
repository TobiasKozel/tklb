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
	#include "../external/xsimd/include/xsimd/config/xsimd_config.hpp"
#endif

namespace tklb {
	/**
	 * Wraps allocation so using a custom memory manager
	 * is an option
	 */
	namespace memory {
		// This can be used to keep track of allocations
		// However only the custom memorymanager keeps track for now
		// stdlib or external allocation functions would need to be wrapped
		size_t Allocated = 0;

		void* (*allocate)(size_t) =
		#ifndef TKLB_MEM_NO_STD
			malloc;
		#else
			nullptr;
		#endif

		void* (*reallocate)(void*, size_t) =
		#ifndef TKLB_MEM_NO_STD
			realloc;
		#else
			nullptr;
		#endif

		void (*deallocate)(void*) =
		#ifndef TKLB_MEM_NO_STD
			free;
		#else
			nullptr;
		#endif

		/**
		 * @brief memcpy wrapper
		 */
		inline void copy(void* dst, const void* src, const size_t size) {
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
		inline void set(void* dst, const unsigned char val, size_t size) {
		#ifdef TKLB_MEM_NO_STD
			auto pointer = reinterpret_cast<unsigned char*>(dst);
			for (size_t i = 0; i < size; i++) {
				pointer[i] = val;
			}
		#else // TKLB_MEM_NO_STD
			memset(dst, val, size);
		#endif // TKLB_MEM_NO_STD
		}


		void* clearallocate(size_t num, size_t size) noexcept {
			const size_t total = size * num;
			void* ptr = allocate(total);
			if (ptr == nullptr) { return nullptr; }
			set(ptr, 0, total);
			return ptr;
		}

		void deallocateAligned(void* ptr) noexcept {
			#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
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
		void* allocateAligned(size_t bytes) noexcept {
			#if !defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
				// TODO replace this
				// This is the aligned allocation routine from xsimd
				// but using the internal allocate function instead
				void* res = 0;
				void* ptr = allocate(bytes + XSIMD_DEFAULT_ALIGNMENT);
				if (ptr != 0 && XSIMD_DEFAULT_ALIGNMENT != 0) {
					// some evil bitwise magic to align to the next block
					res = reinterpret_cast<void*>(
						(reinterpret_cast<size_t>(ptr) &
						~(size_t(XSIMD_DEFAULT_ALIGNMENT - 1))) +
						XSIMD_DEFAULT_ALIGNMENT
					);
					// store away the orignal address needed to free the memory
					*(reinterpret_cast<void**>(res) - 1) = ptr;
				}
				return res;
			#else
				return allocate(bytes);
			#endif // defined(TKLB_NO_SIMD) || defined(TKLB_ALIGNED_MEM)
		}

		/**
		 * @brief Allocate and construct object
		 * @param args Arguments passed to class contructor
		 */
		template <class T, typename ... Args>
		T* create(Args&& ... args) {
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
		void dispose(T* ptr) {
			if (ptr != nullptr) {
				ptr->~T();
				deallocate(ptr);
			}
		}

		/**
		 * Tracer functions also taking file and line as arguments
		 */
	#ifdef TKLB_MEM_TRACE
		void* allocateTrace(size_t size, const char* file, int line) noexcept {
			return allocate(size);
		};

		void* reallocateTrace(void* ptr, size_t size, const char* file, int line) noexcept {
			return reallocate(ptr, size);
		};

		void* clearallocateTrace(size_t num, size_t size, const char* file, int line) noexcept {
			return clearallocate(num, size);
		};

		void deallocateTrace(void* ptr, const char* file, int line) noexcept {
			deallocate(ptr);
		};

		void deallocateAlignedTrace(void* ptr, const char* file, int line) noexcept {
			deallocateAligned(ptr);
		};

		void* allocateAlignedTrace(size_t bytes, const char* file, int line) noexcept {
			return allocateAligned(bytes);
		};

		template <class T, typename ... Args>
		T* createTrace(const char* file, int line, Args&& ... args) {
			return create<T>(std::forward<Args>(args)...);
		}

		template <class T>
		void disposeTrace(T* ptr, const char* file, int line) {
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
	#define TKLB_MALLOC(size)			tklb::memory::allocateTrace(size, __FILE__, __LINE__)
	#define TKLB_FREE(ptr)				tklb::memory::deallocateTrace(ptr, __FILE__, __LINE__)
	#define TKLB_REALLOC(ptr, size) 	tklb::memory::reallocateTrace(ptr, size, __FILE__, __LINE__)
	#define TKLB_CALLOC(num, size) 		tklb::memory::clearallocateTrace(num, size, __FILE__, __LINE__)
	#define TKLB_MALLOC_ALIGNED(size)	tklb::memory::allocateAlignedTrace(size, __FILE__, __LINE__)
	#define TKLB_FREE_ALIGNED(ptr)		tklb::memory::deallocateAlignedTrace(ptr, __FILE__, __LINE__)
	#define TKLB_NEW(T, ...)			tklb::memory::createTrace<T>(__FILE__, __LINE__, ##__VA_ARGS__)
	#define TKLB_DELETE(ptr)			tklb::memory::disposeTrace(ptr, __FILE__, __LINE__)
#else // TKLB_MEM_TRACE
	#define TKLB_MALLOC(size)			tklb::memory::allocate(size)
	#define TKLB_FREE(ptr)				tklb::memory::deallocate(ptr)
	#define TKLB_REALLOC(ptr, size)		tklb::memory::reallocate(ptr, size)
	#define TKLB_CALLOC(num, size) 		tklb::memory::clearallocate(num, size)
	#define TKLB_MALLOC_ALIGNED(size)	tklb::memory::allocateAligned(size)
	#define TKLB_FREE_ALIGNED(ptr)		tklb::memory::deallocateAligned(ptr)
	#define TKLB_NEW(T, ...)			tklb::memory::create<T>(__VA_ARGS__)
	#define TKLB_DELETE(ptr)			tklb::memory::dispose(ptr)
#endif // TKLB_MEM_TRACE


/**
 *
 * Force all other standart allocation operators to use the
 * ones provided by tklb. This is potentially dangerous
 * when linkin in libraries and passing memory to them.
 *
 */
#ifdef TKLB_MEM_OVERLOAD_ALL
	#define malloc(size)		TKLB_MALLOC(size)
	#define free(ptr)			TKLB_FREE(ptr)
	#define realloc(ptr, size)	TKLB_REALLOC(ptr, size)
	#define calloc(num, size)	TKLB_CALLOC(num, size)

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
