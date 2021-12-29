/**
 * Wrap assertions
 */

#ifndef TKLB_ASSERT

#ifdef TKLB_NO_ASSERT
	#define TKLB_ASSERT(condition);
	#define TKLB_ASSERT_STATE(condition);
#else
	#ifdef _MSC_VER
		#include <cassert>
		#define TKLB_ASSERT(condition) assert(condition);
		// #include <intrin.h>
		// #define TKLB_ASSERT(condition) if (!(condition)) { __debugbreak(); }
	#else
		#include <cassert>
		#define TKLB_ASSERT(condition) assert(condition);
		// #include <signal.h>
		// #define TKLB_ASSERT(condition) if (!(condition)) { raise(SIGTRAP); }
	#endif

#endif // TKLB_NO_ASSERT

#endif // TKLB_ASSERT

#ifndef TKLB_ASSERT_STATE
	/**
	 * @brief This can be used to write some additional code
	 * only needed when doing assertions
	 */
	#define TKLB_ASSERT_STATE(condition) condition;
#endif // TKLB_ASSERT_STATE
