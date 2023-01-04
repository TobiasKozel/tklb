#ifndef _TKLB_NEW
#define _TKLB_NEW

	#ifndef TKLB_NO_STDLIB
		#include <new>
	#else // TKLB_NO_STDLIB
		inline void* operator new(std::size_t, void* __p) { return __p; }
	#endif // TKLB_NO_STDLIB

#endif // _TKLB_NEW
