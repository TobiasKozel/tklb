#define TKLB_IMPL
#define TKLB_CUSTOM_MALLOC

#include "../src/util/TAssert.h"
#include "../src/memory/TFixedPool.hpp"
#include "../src/memory/TMemoryCheck.hpp"
#include "../src/util/TMath.hpp"


static constexpr size_t poolSize = 1024 * 1024 * 64;
char* poolMamory = new char[poolSize];
tklb::memory::FixedPool pool = { poolMamory, poolSize };

void* tklb_malloc(size_t bytes) {
	using MagicBlock = tklb::memory::check::MagicBlock;
	return MagicBlock::construct(
		pool.allocate(bytes + MagicBlock::sizeNeeded()),
		bytes
	);
}

void tklb_free(void* ptr) {
	using MagicBlock = tklb::memory::check::MagicBlock;
	if (ptr == nullptr) { return; }
	auto result = MagicBlock::check(ptr);
	if (result.overrun || result.underrun) {
		TKLB_ASSERT(false)
	}

	if (!result.underrun) {
		pool.deallocate(result.ptr);
	}
}

/**
 * @brief macro to bail on non zero exit codes
 *
 */
#define returnNonZero(val) 						\
{												\
	int ret = val;								\
	if(ret != 0) { return ret; }				\
}

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
	if (pool.allocated() != 0) {
		return 11111;
	}
	return ret;
}
