/**
 * @file TAssert.h
 * @author Tobias Kozel
 * @brief Macro for assertions to allow different ways of handling them
 * @version 0.1
 * @date 2023-05-05
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _TKLB_ASSERT
#define _TKLB_ASSERT

// In case there's a user defined assert
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

#endif // _TKLB_ASSERT
