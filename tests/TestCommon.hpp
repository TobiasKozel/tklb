#define TKLB_MEM_TRACE
#define TKLB_MEM_MONKEY_PATCH
#ifdef _WIN32
	#define TKLB_ASSERT_SEGFAULT
#endif
#include "../src/memory/TMemory.hpp"
#include <cmath>
#include "../src/util/TTimer.hpp"

using namespace tklb;
using namespace std;

int retn0;
#define returnNonZero(val) retn0 = val; if(retn0 != 0) { return retn0; }

bool close(float a, float b, float epsylon = 0.01) {
	if (std::abs(a - b) < epsylon) {
		return true;
	} else {
		return false; // Easy to set a breakpoint
	}
}

int test();

int main() {
	int ret = test();

	return ret;
}
