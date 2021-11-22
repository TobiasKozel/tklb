
#ifndef TKLBZ_AUDIOFILE
	#include "./TWaveFile.hpp"

	namespace tklb { namespace wave { namespace _ {
		void* drwaveMalloc(size_t size, void* userData) {
			return TKLB_MALLOC(size);
		}

		void drwaveFree(void* ptr, void* userData) {
			TKLB_FREE(ptr);
		}
	} } } // tklb::wave::_
#endif


#define DR_WAV_IMPLEMENTATION
#include "../../../external/dr_wav.h"

