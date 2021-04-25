#ifndef TKLBZ_MEMORY_UTIL
#define TKLBZ_MEMORY_UTIL

#ifndef TKLB_MEM_NO_STD
	#include <cstring>
	#include <stdlib.h>
#endif

namespace tklb { namespace memory {

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

} } // namesoace tklb::memory

#endif // TKLBZ_MEMORY_UTIL
