#ifndef _TKLB_TIMER
#define _TKLB_TIMER

#include <string>
#include <chrono>
#include <ctime>
#include <stdio.h>
#include "./TPrint.h"

namespace tklb {

	class SectionTimer {
	public:
		enum Unit {
			Miliseconds,
			Microseconds,
			Nanoseconds
		} mUnit;

	private:
		const char* mMessage;

		size_t mDivider;

		#ifdef _MSC_VER
			using Time = std::chrono::time_point<std::chrono::steady_clock>;
		#else
			using Time = std::chrono::system_clock::time_point;
		#endif

		Time mStart;

	public:

		static Time current() {
			return std::chrono::high_resolution_clock::now();
		}

		static size_t getMsSince(const Time& t) {
			return std::chrono::duration_cast<std::chrono::milliseconds>(current() - t).count();
		}

		static size_t getUsSince(const Time& t) {
			return std::chrono::duration_cast<std::chrono::microseconds>(current() - t).count();
		}

		static size_t getNsSince(const Time& t) {
			return std::chrono::duration_cast<std::chrono::nanoseconds>(current() - t).count();
		}

		SectionTimer(const char* message = "", Unit unit = Microseconds, size_t divider = 1) {
			mMessage = message;
			mUnit = unit;
			mStart = current();
			mDivider = divider;
		}

		~SectionTimer() {
			if(mUnit == Nanoseconds) {
				TKLB_PRINT("%s\t%zu\tnanoseconds\n", mMessage, getNsSince(mStart) / mDivider)
			}
			if (mUnit == Microseconds) {
				TKLB_PRINT("%s\t%zu\tmicroseconds\n", mMessage, getUsSince(mStart) / mDivider)
			}
			if (mUnit == Miliseconds) {
				TKLB_PRINT("%s\t%zu\tmilliseconds\n", mMessage, getMsSince(mStart) / mDivider)
			}
		}
	};

	class SectionClock {
		const char* mMessage;
		std::clock_t mStart;

	public:
		SectionClock(const char* message = "") {
			mMessage = message;
			mStart = std::clock();
		}

		~SectionClock() {
			std::clock_t now = std::clock();
			TKLB_PRINT("%s\t%i\tCycles\n", mMessage, int(now - mStart))
		}
	};

} // namespace

#endif // TKLB_TIMER
