#ifndef TKLB_ASSERT

#ifdef TKLB_NO_ASSERT
	#define TKLB_ASSERT(condition)
#else
	#include <cassert>
	#define TKLB_ASSERT(condition) assert((condition))
#endif // TKLB_NO_ASSERT

#endif // TKLB_ASSERT
