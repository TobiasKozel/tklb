#ifndef _TKLB_NEW
#define _TKLB_NEW

	#ifndef TKLB_NO_STDLIB
		#include <new>
	#else // TKLB_NO_STDLIB
		// TODO Need a way to avoid defining this if it already is from the std
		#ifndef _NEW
			#include "../types/TTypes.hpp"
			inline void* operator new(unsigned long, void* __p) { return __p; }
		#endif
	#endif // TKLB_NO_STDLIB

#endif // _TKLB_NEW
