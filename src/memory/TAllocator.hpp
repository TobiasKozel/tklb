#ifndef _TKLB_ALLOCATOR
#define _TKLB_ALLOCATOR

#include <stddef.h>			// size_t
#include <limits>			// addressable space

#include "./TMemory.hpp"	// tklb_malloc

#ifdef TKLB_USE_PROFILER
	#include "../util/TProfiler.hpp"
#else
	#define TKLB_PROFILER_MALLOC_L(ptr, size, name)
	#define TKLB_PROFILER_FREE_L(ptr, name)
#endif // TKLB_USE_PROFILER

namespace tklb {
	/**
	 * @brief Names to the allocator are passed by type
	 */
	struct DefaultAllocatorName { static constexpr const char* Name = "Default Allocator"; };

	/**
	* @brief Basic allocator forwarding to the standard library
	* @tparam T Element to allocate
	* @tparam NAME In case the allocation are tracked, the allocator can have a name
	*/
	template <class T = unsigned char, class NAME = DefaultAllocatorName>
	struct DefaultAllocator {
		static constexpr size_t AddressSpace = std::numeric_limits<size_t>::max() / sizeof(T);
		typedef T value_type;

		DefaultAllocator() = default;
		DefaultAllocator(const DefaultAllocator&) = default;

		template <class U, class NAME2>
		DefaultAllocator(const DefaultAllocator<U, NAME2>&) { }

		T* allocate(size_t n) noexcept {
			#ifndef TKLB_RELEASE
				if (AddressSpace < n) { return nullptr; } // ! allocation too large
			#endif

			const auto bytes = n * sizeof(T);
			if (auto ptr = static_cast<T*>(tklb_malloc(bytes))) {
				TKLB_PROFILER_MALLOC_L(ptr, bytes, NAME::Name)
				return ptr;
			}
			return nullptr;
		}

		/**
		* @brief Free memory
		*
		* @param ptr pointer
		* @param n Element count to be freed, not really needed for anything besides stats
		*/
		void deallocate(T* ptr, size_t n) noexcept {
			TKLB_PROFILER_FREE_L(ptr, NAME::Name)
			tklb_free(ptr);
		}

	};

	/**
	 * @brief I don't even know what this does, but it has to be here for some classes using
	 */
	// template <class T, class U, class J>
	// bool operator==(const DefaultAllocator<T, J>&, const DefaultAllocator<U, J>&) {
	// 	return true;
	// }

	// template <class T, class U, class J>
	// bool operator!=(const DefaultAllocator<T, J>&, const DefaultAllocator<U, J>&) {
	// 	return false;
	// }
}

#endif // _TKLB_ALLOCATOR
