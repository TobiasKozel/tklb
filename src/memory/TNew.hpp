#ifndef _TKLB_NEW
#define _TKLB_NEW

	#ifndef TKLB_NO_STDLIB
		#include <new>
	#else // TKLB_NO_STDLIB
		#include "../types/TTypes.hpp"
		inline void* operator new(unsigned long, void* __p) { return __p; }
	#endif // TKLB_NO_STDLIB

#endif // _TKLB_NEW
