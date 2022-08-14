#ifndef TKLB_ASSERT

#ifdef TKLB_NO_ASSERT
	#define TKLB_ASSERT(condition);
	#define TKLB_ASSERT_STATE(condition);
#else
	#include <cassert>
	// #define TKLB_ASSERT(condition) assert(condition);

	// Below are function to explictly break with an attached debugger, which is useful
	// to continue after an assert.
	#ifdef _MSC_VER
		#include <intrin.h>
		#define TKLB_ASSERT(condition) if (!(condition)) { __debugbreak(); }
	#else
		#include <signal.h>
		#define TKLB_ASSERT(condition) if (!(condition)) { raise(SIGTRAP); }
	#endif

#endif // TKLB_NO_ASSERT

#endif // TKLB_ASSERT

#ifndef TKLB_ASSERT_STATE
	/**
	 * @brief This can be used to write some additional code like define a variable.
	 * only needed when doing assertions
	 */
	#define TKLB_ASSERT_STATE(condition) condition;
#endif // TKLB_ASSERT_STATE
