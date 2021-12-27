#ifndef _TKLB_ALLOCATOR
#define _TKLB_ALLOCATOR

#include "../util/TAssert.h"

#include <stddef.h>
#include <limits>
#include <stdlib.h>

#ifndef TKLB_TRACK_ALLOCATE
	#define TKLB_TRACK_ALLOCATE(ptr, size)
	#define TKLB_TRACK_FREE(ptr, size)
#endif

namespace tklb {
	template <class T>
	struct StdAllocator {
		typedef T value_type;

		StdAllocator() = default;
		StdAllocator(const StdAllocator&) = default;
		template <class U>
		StdAllocator(const StdAllocator<U>&) { }

		T* allocate(size_t n) noexcept {
			if (n > std::numeric_limits<size_t>::max() / sizeof(T)) {
				return nullptr;
			}
			const auto bytes = n * sizeof(T);
			if (auto ptr = static_cast<T*>(malloc(bytes))) {
				TKLB_TRACK_ALLOCATE(ptr, bytes);
				return ptr;
			}

			return nullptr;
		}

		void deallocate(T* ptr, size_t n) noexcept {
			TKLB_TRACK_FREE(ptr, sizeof(T) * n);
			free(ptr);
		}
	};
}

#endif // _TKLB_ALLOCATOR
