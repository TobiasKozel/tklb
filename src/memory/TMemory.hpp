#ifndef _TKLB_MEMORY
#define _TKLB_MEMORY

#include "../types/TTypes.hpp"

#ifndef TKLB_NO_STDLIB
	#include <stdlib.h>		// malloc & free
	#include <cstring>		// memcpy & memset
	#include <algorithm>	// fill_n
#endif

#ifdef TKLB_USE_PROFILER
	#include "../util/TProfiler.hpp"
#else
	#define TKLB_PROFILER_MALLOC(ptr, size)
	#define TKLB_PROFILER_FREE(ptr)
#endif

/**
 * @brief Main allocation function.
 *        Use TKLB_MALLOC which allows tracking memory.
 *        If need be this can be implemented by the user
 *        to redirect allocations.
 * @param bytes Bytes to allocate unaligned
 * @return void* Resulting memory
 */
void* tklb_malloc(tklb::SizeT bytes);

/**
 * @brief Main free function. Use TKLB_FREE instead which allows tracking
 * @param ptr
 */
void tklb_free(void* ptr);

#if !defined(TKLB_NO_STDLIB) && defined(TKLB_IMPL) && !defined(TKLB_CUSTOM_MALLOC)
	#ifdef TKLB_MEMORY_CHECK
		#include "./TMemoryCheck.hpp"

		void* tklb_malloc(SizeT bytes) {
			using MagicBlock = tklb::memory::check::MagicBlock;
			return MagicBlock::construct(malloc(bytes + MagicBlock::sizeNeeded()), bytes);
		}

		void tklb_free(void* ptr) {
			if (ptr == nullptr) { return; }
			using MagicBlock = tklb::memory::check::MagicBlock;
			auto result = MagicBlock::check(ptr);
			if (result.overrun || result.underrun) {
				TKLB_ASSERT(false)
			}

			if (!result.underrun) {
				free(result.ptr);
			}
		}
	#else // TKLB_MEMORY_CHECK
		void* tklb_malloc(tklb::SizeT bytes) {
			return malloc(bytes);
		}
		void tklb_free(void* ptr) {
			free(ptr);
		}
	#endif // TKLB_MEMORY_CHECK
#endif // memory impl

namespace tklb { namespace memory {
	/**
	 * @brief Acts like new. Allocate and construct object.
	 * @param args Arguments passed to class contructor
	 */
	template <class T, typename ... Args>
	static T* create(Args&& ... args) {
		constexpr auto size = sizeof(T);
		void* ptr = tklb_malloc(size);
		if (ptr == nullptr) { return nullptr; }
		// TODO test if this even works without std::forward
		new (ptr) T((args)...);
		return reinterpret_cast<T*>(ptr);
	}

	/**
	 * @brief Acts like delete. Destroy the object and dispose the memory.
	 */
	template <class T>
	static void dispose(T* ptr) {
		if (ptr == nullptr) { return; }
		ptr->~T();
		tklb_free(ptr);
	}

	/**
	 * @brief memcpy wrapper
	 */
	static inline void copy(void* dst, const void* src, const SizeT size) {
	#ifdef TKLB_NO_STDLIB
		auto source = reinterpret_cast<const unsigned char*>(src);
		auto destination = reinterpret_cast<unsigned char*>(dst);
		for (SizeT i = 0; i < size; i++) {
			destination[i] = source[i];
		}
	#else // TKLB_MEM_NO_STD
		memcpy(dst, src, size);
	#endif // TKLB_MEM_NO_STD
	}

	/**
	 * @brief Own (hopefully) safe string copy
	 *
	 * @param dst
	 * @param src
	 * @param size Size of the dst buffer.
	 * @param terminate Whether the last character in the destination will be '\0' terminated for safety.
	 */
	static inline void stringCopy(char* dst, const char* src, SizeT size, bool terminate = true) {
		for (SizeT i = 0; i < size; i++) {
			dst[i] = src[i];
			if (src[i] == '\0') { return; }
		}
		if (terminate) {
			// run over the last char to make sure it's terminated
			dst[size - 1] = '\0';
		}
	}

	/**
	 * @brief Set memory, similar to std::fill_n
	 *
	 * @tparam T Element Type
	 * @param dst memory
	 * @param elements
	 * @param val
	 */
	template <typename T>
	static inline void set(T* dst, SizeT elements, const T& val) {
		#ifdef TKLB_NO_STDLIB
			for (SizeT i = 0; i < elements; i++) {
				dst[i] = val;
			}
		#else
			std::fill_n(dst, elements, val);
		#endif
	}

	/**
	 * @brief Zero the memory
	 *
	 * @param dst
	 * @param bytes
	 */
	static inline void zero(void* dst, SizeT bytes) {
		const auto ptr = reinterpret_cast<char*>(dst);
		set<char>(ptr, bytes, 0);
	}
} } // tklb::memory


#ifndef TKLB_MALLOC // TODO TKLB memory tracer should take a detour
	#define TKLB_MALLOC(size)		tklb_malloc(size);
	#define TKLB_FREE(ptr)			tklb_free(ptr);
	#define TKLB_CALLOC(num, size) 	notImplemented; // TODO TKLB
	#define TKLB_NEW_PARAM(T, ...)	tklb::memory::create<T>(__VA_ARGS__);
	#define TKLB_NEW(T)				tklb::memory::create<T>();
	#define TKLB_DELETE(T, ptr)		tklb::memory::dispose<T>(ptr);
#endif // TKLB_MALLOC

#endif // _TKLB_MEMORY
