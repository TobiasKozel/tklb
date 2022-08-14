#define TKLB_MEMORY_CHECK
#define TKLB_IMPL
#define TKLB_ASSERT(val) ASSERT_RESULT = val;
bool ASSERT_RESULT;

#include "../src/memory/TMemory.hpp"

int main() {
	{
		int* overrun = (int*) tklb_malloc(sizeof(int) * 4);
		overrun[4] = 1;
		ASSERT_RESULT = true;
		tklb_free(overrun);

		if (ASSERT_RESULT != false) {
			return 1;
		}
	}

	{
		int* underrun = (int*) tklb_malloc(sizeof(int) * 4);
		underrun[-1] = 1;
		ASSERT_RESULT = true;
		tklb_free(underrun);

		if (ASSERT_RESULT != false) {
			return 2;
		}
	}

	return 0;
}
