#ifndef TKLB_ASSERT

#if defined(TKLB_NO_ASSERT) || defined (TKLB_NO_ASSERT)
	#define TKLB_ASSERT(condition);
	#define TKLB_ASSERT_STATE(condition);
#else
	#ifdef TKLB_ASSERT_BREAK
		#ifdef _MSC_VER
			#include <intrin.h>
			#define TKLB_ASSERT(condition) if (!(condition)) { __debugbreak(); }
		#else
			#include <signal.h>
			#define TKLB_ASSERT(condition) if (!(condition)) { raise(SIGTRAP); }
		#endif
	#else // TKLB_ASSERT_BREAK
		#include <cassert>
		#define TKLB_ASSERT(condition) assert(condition);
	#endif // TKLB_ASSERT_BREAK


#endif // TKLB_NO_ASSERT

#endif // TKLB_ASSERT

#ifndef TKLB_ASSERT_STATE
	/**
	 * @brief This can be used to write some additional code like define a variable.
	 * only needed when doing assertions
	 */
	#define TKLB_ASSERT_STATE(condition) condition;
#endif // TKLB_ASSERT_STATE
