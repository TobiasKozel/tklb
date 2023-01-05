#define TKLB_IMPL

bool heapCorruption = false;

#define TKLB_CUSTOM_MALLOC
static constexpr unsigned int poolSize = 1024 * 1024 * 64;
char* poolMemory = new char[poolSize];

#include "../src/memory/TFixedPool.hpp"
tklb::memory::FixedPool pool = { poolMemory, poolSize };

#include "../src/util/TAssert.h"
#include "../src/memory/TMemoryCheck.hpp"

void* tklb_malloc(tklb::SizeT bytes) {
	using MagicBlock = tklb::memory::check::MagicBlock;
	auto ptr = pool.allocate(bytes + MagicBlock::sizeNeeded());
	auto ptr2 = MagicBlock::construct(ptr, bytes);
	return ptr2;
}

void tklb_free(void* ptr) {
	using MagicBlock = tklb::memory::check::MagicBlock;
	if (ptr == nullptr) { return; }
	auto result = MagicBlock::check(ptr);
	if (result.overrun || result.underrun) {
		heapCorruption = true;
	}
	pool.deallocate(result.ptr);
}

#ifdef TKLB_NO_STDLIB
	void tklb_print(int level, const char* message) { }
#endif

/**
 * @brief macro to bail on non zero exit codes
 *
 */
#define returnNonZero(val) 						\
{												\
	int ret = val;								\
	if(ret != 0) { return ret; }				\
}

#include "../src/util/TMath.hpp"

/**
 * @brief Check if float values are close
 *
 * @param a
 * @param b
 * @param epsylon
 * @return true Within epsylon
 * @return false Outside epsylon
 */
bool close(float a, float b, float epsylon = 0.01) {
	if (tklb::abs(a - b) < epsylon) {
		return true;
	} else {
		return false; // Easy to set a breakpoint
	}
}

/**
 * @brief Will be defined in the corresponding test
 *
 * @return int 0 on success
 */
int test();

/**
 * @brief Hijack the main function
 *
 * @return int
 */
int main() {
	int ret = test();
	// TODO some logic handling and maybe pretty test error strings
	if (heapCorruption) {
		return 202; // fail test in case heap corruptions occured.
	}
	if (pool.allocated() != 0) {
		return 222; // fail the test for dangling allocations
	}
	delete[] poolMemory;
	return ret;
}
