#ifndef TKLB_TIMER
#define TKLB_TIMER

#include <string>
#include <chrono>
#include <iostream>

namespace tklb {

class SectionTimer {
	const char* mMessage;
	bool mNanoseconds;
	std::chrono::time_point<std::chrono::system_clock> mStart;

public:
	SectionTimer(const char* message = "", bool nanoseconds = true) {
		mMessage = message;
		mNanoseconds = nanoseconds;
		mStart = std::chrono::high_resolution_clock::now();
	}

	~SectionTimer() {
		auto now = std::chrono::high_resolution_clock::now();
		if(mNanoseconds) {
			auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(now - mStart);
			std::cout << mMessage << dur.count() << " ns\n";
		} else {
			auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(now - mStart);
			std::cout << mMessage << dur.count() << " ms\n";
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
		std::cout << mMessage << (now - mStart) << " cycles\n";
	}
};

} // namespace

#endif // TKLB_TIMER
