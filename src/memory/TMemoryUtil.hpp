#ifndef _TKLB_MEMORY_UTIL
#define _TKLB_MEMORY_UTIL

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

	static inline void stringCopy(char* dst, const char* src, size_t size, bool terminate = true) {
		for (size_t i = 0; i < size; i++) {
			dst[i] = src[i];
			if (src[i] == '\0') { return; }
		}
		if (terminate) {
			// run over the last char to make sure it's terminated
			dst[size - 1] = '\0';
		}
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

#endif // _TKLB_MEMORY_UTIL
