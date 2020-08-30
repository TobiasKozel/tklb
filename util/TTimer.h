#ifndef TKLB_TIMER
#define TKLB_TIMER

#include <string>
#include <chrono>
#include <iostream>
#include <ctime>

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
	#ifdef _MSC_VER
		using Time = std::chrono::time_point<std::chrono::steady_clock>;
	#else
		using Time = std::chrono::system_clock::time_point;
	#endif
	Time mStart;

public:
	SectionTimer(const char* message = "", Unit unit = Microseconds) {
		mMessage = message;
		mUnit = unit;
		mStart = std::chrono::high_resolution_clock::now();
		auto test = std::chrono::high_resolution_clock::now();
	}

	~SectionTimer() {
		Time now = std::chrono::high_resolution_clock::now();
		if(mUnit == Nanoseconds) {
			auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(now - mStart);
			std::cout << mMessage << " " << dur.count() << " ns\n";
		}
		if (mUnit == Microseconds) {
			auto dur = std::chrono::duration_cast<std::chrono::microseconds>(now - mStart);
			std::cout << mMessage << " " << dur.count() << "  microseconds\n";
		}
		if (mUnit == Miliseconds) {
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStart);
			std::cout << mMessage << " " << dur.count() << "  ms\n";
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
		std::cout << mMessage << " " << (now - mStart) << " cycles\n";
	}
};

} // namespace

#endif // TKLB_TIMER
