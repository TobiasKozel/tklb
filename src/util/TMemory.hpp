#ifndef TKLBZ_MEMORY
#define TKLBZ_MEMORY

#include <stddef.h>
#include <utility>

#ifndef TKLB_MEM_NO_STD
	#include <cstring>
	#include <stdlib.h>
#endif

#ifndef TKLB_MEM_OVERLOAD_ALL
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

#pragma region Memory tracing stuff
		/**
		 * Tracer functions also taking file and line as arguments
		 */
	#ifdef TKLB_MEM_TRACE

		constexpr char TKLB_MAGIC_STRING[] = "tklbend";
		constexpr char TKLB_MAGIC_BACKUP_STRING[] = "tklback";
		/**
		 * @brief Struct inserted at the end of every allocation.
		 */
		struct MagicBlock {
			char magic[sizeof(TKLB_MAGIC_STRING)];
			char padding[1000]; // padding space used to protect the data after
			char magicBackup[sizeof(TKLB_MAGIC_BACKUP_STRING)];
			const char* file; // source file
			int line; // line in source file
			void* ptr; // the start of the allocated mem
			size_t size;
			MagicBlock(const char* f, int l, void* p, size_t s) {
				file = f;
				line = l;
				ptr = p;
				size = s;
				memory::set(magic, 0, sizeof(magic));
				memory::copy(magic, TKLB_MAGIC_STRING, sizeof(TKLB_MAGIC_STRING));
				memory::copy(magicBackup, TKLB_MAGIC_BACKUP_STRING, sizeof(TKLB_MAGIC_BACKUP_STRING));
			}

			/**
			 * @brief Placement new at the end of the allocation
			 */
			static void construct(void* ptr, size_t s, const char* f, int l) {
				if (ptr == nullptr) { return; }
				MagicBlock* loc = reinterpret_cast<MagicBlock*>(static_cast<char*>(ptr) + s);
				MagicBlock* test = new (loc) MagicBlock(f, l, ptr, s);
			}

			static bool compare(const char* a, const char* b, size_t s) {
				for (size_t i = 0; i < s; i++) {
					if (a[i] != b[i]) {
						return false;
					}
				}
				return true;
			}

			/**
			 * @brief Since we don't save the size of the allocations,
			 * we need to find the magic block by simply going through the memory.
			 */
			static void check(void* start) {
				char* pointer = static_cast<char*>(start);
				// 1 gb of search space will most likely result in a segfault
				// which means the Block isn't there any more
				for (size_t i = 0; i < 1024 * 1024 * 1024; i++) {
					if (!compare(pointer + i, TKLB_MAGIC_STRING, sizeof(TKLB_MAGIC_STRING))) {
						if (!compare(pointer + i, TKLB_MAGIC_BACKUP_STRING, sizeof(TKLB_MAGIC_BACKUP_STRING))) {
							continue;
						}
						// found partially overrun block
						MagicBlock* badBlock = reinterpret_cast<MagicBlock*>(
							pointer + i - sizeof(TKLB_MAGIC_STRING) - sizeof(padding)
						);
						TKLB_ASSERT(false)
					}
					// found a block
					MagicBlock* block = reinterpret_cast<MagicBlock*>(pointer + i);
					// Make sure we found the block that matches the allocation
					if (block->ptr == start) { return; }
					// Block is missing completely
					TKLB_ASSERT(false)
				}
				// Block is missing or out of the search range
				TKLB_ASSERT(false)
			}
		};

		static inline void* allocateTrace(size_t size, const char* file, int line) noexcept {
			void* mem = allocate(size + sizeof(MagicBlock));
			MagicBlock::construct(mem, size, file, line);
			return mem;
		};

		static inline void* reallocateTrace(void* ptr, size_t size, const char* file, int line) noexcept {
			void* mem = reallocate(ptr, size + sizeof(MagicBlock));
			MagicBlock::construct(mem, size, file, line);
			return mem;
		};

		static inline void* clearallocateTrace(size_t num, size_t size, const char* file, int line) noexcept {
			// how much space clearallocate will allocate
			size_t total = num * size;
			// the amount of elements needed
			size_t numNeeded = ((total + sizeof(MagicBlock)) / size) + 1;
			void* mem = clearallocate(numNeeded, size);
			MagicBlock::construct(mem, total, file, line);
			return mem;
		};

		static inline void deallocateTrace(void* ptr, const char* file, int line) noexcept {
			if (ptr == nullptr) { return; }
			MagicBlock::check(ptr);
			deallocate(ptr);
		};

		static inline void deallocateAlignedTrace(void* ptr, const char* file, int line) noexcept {
			if (ptr == nullptr) { return; }
			MagicBlock::check(ptr);
			deallocateAligned(ptr);
		};

		static inline void* allocateAlignedTrace(const char* file, int line, size_t size, size_t align = DEFAULT_ALIGN) noexcept {
			void* mem = allocateAligned(size + sizeof(MagicBlock), align);
			MagicBlock::construct(mem, size, file, line);
			return mem;
		};

		template <class T, typename ... Args>
		static inline T* createTrace(const char* file, int line, Args&& ... args) {
			void* mem = allocate(sizeof(T) + sizeof(MagicBlock));
			if (mem == nullptr) { return nullptr; }
			MagicBlock::construct(mem, sizeof(T), file, line);
			new (mem) T(std::forward<Args>(args)...);
			return reinterpret_cast<T*>(mem);
		}

		template <class T>
		static inline void disposeTrace(T* ptr, const char* file, int line) {
			if (ptr == nullptr) { return; }
			MagicBlock::check(ptr);
			ptr->~T();
			deallocate(ptr);
		}
	#endif // TKLB_MEM_TRACE
#pragma endregion

	} // namespace memory
} // namespace tklb

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

	// Gets rif of redefinition warnings
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
