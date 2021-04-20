#define TKLB_MEM_TRACE
#define TKLB_MEM_OVERLOAD_ALL
#ifdef _WIN32
	#define TKLB_ASSERT_SEGFAULT
#endif
#include "../src/memory/TMemoryManager.hpp"
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
	const size_t size = 1024 * 1024 * 300; // 300MB
	void* mem = memory::std_allocate(size);
	memory::manager::use(mem, size);
	int ret = test();
	memory::manager::restore();
	memory::std_deallocate(mem);
	if (tklb::memory::Allocated != 0) {
		TKLB_ASSERT(false)
		return 100;
	}
	return ret;
}
