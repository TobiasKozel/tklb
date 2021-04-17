/**
 * Wrap assertions, just in case
 */

#ifndef TKLB_ASSERT
#ifdef TKLB_NO_ASSERT
	#define TKLB_ASSERT(condition);
	#define TKLB_ASSERT_STATE(condition);
#else
	#ifdef TKLB_ASSERT_SEGFAULT
		// Triggers a segfault. Dumbest way to break in the debugger.
		#define TKLB_ASSERT(condition) if (condition) { } else { reinterpret_cast<int*>(0)[0] = 0; };
	#else
		#include <cassert>
		#define TKLB_ASSERT(condition) assert(condition);
	#endif


	/**
	 * @brief This can be used to write some additional code
	 * only needed when doing assertions
	 */
	#define TKLB_ASSERT_STATE(condition) condition;

#endif // TKLB_NO_ASSERT
#endif // TKLB_ASSERT
