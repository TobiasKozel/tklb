#ifndef _TKLB_ALLOCATOR
#define _TKLB_ALLOCATOR

#include "./TMemory.hpp"	// tklb_malloc
#include "../util/TLimits.hpp"

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
		static constexpr SizeT AddressableElements = limits::max<SizeT>::value / sizeof(T);
		typedef T value_type;

		constexpr DefaultAllocator() = default;
		constexpr DefaultAllocator(const DefaultAllocator&) = default;

		template <class U, class NAME2>
		constexpr DefaultAllocator(const DefaultAllocator<U, NAME2>&) { }

		T* allocate(SizeT n) const noexcept {
			#ifndef TKLB_RELEASE
				if (AddressableElements < n) { return nullptr; } // ! allocation too large
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
		void deallocate(T* ptr, SizeT n) const noexcept {
			(void) n;
			TKLB_PROFILER_FREE_L(ptr, NAME::Name)
			tklb_free(ptr);
		}

	};
}

#endif // _TKLB_ALLOCATOR
