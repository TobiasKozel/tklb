/**
 * @file TTimer.hpp
 * @author Tobias Kozel
 * @brief Simple wrapper around the std chrono stuff to make measuring easier
 * @version 0.1
 * @date 2022-08-05
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef _TKLB_TIMER
#define _TKLB_TIMER

#include <chrono>
#include <ctime>
#include <stdio.h>

namespace tklb {

	/**
	 * @brief Measures time from construction to dedstruction and
	 *        prints the result in the given unit.
	 */
	class SectionTimer {
	public:
		enum class Unit {
			Miliseconds,
			Microseconds,
			Nanoseconds,
			Cycles
		} mUnit;

	private:
		const char* mMessage;
		size_t mDivider;
		using Time = std::chrono::time_point<std::chrono::steady_clock>;
		Time mStart;

	public:

		static inline Time current() {
			return std::chrono::steady_clock::now();
		}

		static inline size_t getMsSince(const Time& t) {
			return std::chrono::duration_cast<std::chrono::milliseconds>(current() - t).count();
		}

		static inline  size_t getUsSince(const Time& t) {
			return std::chrono::duration_cast<std::chrono::microseconds>(current() - t).count();
		}

		static inline size_t getNsSince(const Time& t) {
			return std::chrono::duration_cast<std::chrono::nanoseconds>(current() - t).count();
		}

		SectionTimer(const char* message = "", Unit unit = Unit::Microseconds, size_t divider = 1) {
			mMessage = message;
			mUnit = unit;
			mDivider = divider;
			mStart = current();
		}

		~SectionTimer() {
			if(mUnit == Unit::Nanoseconds) {
				printf("%s\t%zu\tnanoseconds\n", mMessage, getNsSince(mStart) / mDivider);
			}
			if (mUnit == Unit::Microseconds) {
				printf("%s\t%zu\tmicroseconds\n", mMessage, getUsSince(mStart) / mDivider);
			}
			if (mUnit == Unit::Miliseconds) {
				printf("%s\t%zu\tmilliseconds\n", mMessage, getMsSince(mStart) / mDivider);
			}
			if (mUnit == Unit::Cycles) {
				printf("%s\t%zu\tmilliseconds\n", mMessage,  (current() - mStart).count() / mDivider);
			}
		}
	};
} // namespace

#endif // TKLB_TIMER
