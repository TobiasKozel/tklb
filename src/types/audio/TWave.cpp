#include "./TWave.hpp"

namespace tklb { namespace wave { namespace _ {
	void* drwaveMalloc(size_t size, void* userData) {
		(void) userData;
		return TKLB_MALLOC(size);
	}

	void drwaveFree(void* ptr, void* userData) {
		(void) userData;
		TKLB_FREE(ptr);
	}
} } } // tklb::wave::_

#define DR_WAV_IMPLEMENTATION
#include "../../../external/dr_wav.h"

