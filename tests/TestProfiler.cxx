
#define TKLB_IMPL
#define TKLB_USE_PROFILER
#include "../src/util/TProfiler.hpp"

void func(double value) {
	TKLB_PROFILER_SCOPE_NAMED("funcscope");
	TKLB_PROFILER_PLOT("val", value);
}

int main() {
	TKLB_PROFILER_THREAD_NAME("main thread");
	TKLB_PROFILER_MESSAGE_L("Profile start");
	for (int i = 0; i < 100; i++) {
		TKLB_PROFILER_FRAME_MARK();
		const auto size = 128;
		auto testMem = new char[size];
		TKLB_PROFILER_MALLOC_L(testMem, size, "Test Alloc");
		func(i);
		TKLB_PROFILER_FREE_L(testMem, "Test Alloc");
	}
	return 0;
}
