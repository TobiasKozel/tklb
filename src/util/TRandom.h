/**
 * @file TRandom.h
 * @author Tobias Kozel
 * @brief Linear Congruential Generator
 * @version 0.1
 * @date 2023-08-06
 *
 * @copyright Copyright (c) 2023
 *
 */
 #ifndef _TKLB_RANDOM
 #define _TKLB_RANDOM

namespace tklb { class random {
	unsigned long mState;
public:
	random(unsigned long seed) : mState(seed) { }
	unsigned long next() {
		mState = (mState * 1103515245 + 12345) % 2147483648;
		return mState;
	}
}; } // tklb::random
 #endif // _TKLB_RANDOM
