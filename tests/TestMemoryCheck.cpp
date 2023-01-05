// #define TKLB_MEMORY_CHECK

#include "./TestCommon.hpp"

int test() {
	{
		int* overrun = (int*) tklb_malloc(sizeof(int) * 4);
		overrun[4] = 1;
		tklb_free(overrun);

		if (!heapCorruption) {
			return 1;
		}
		heapCorruption = false;
	}

	{
		int* underrun = (int*) tklb_malloc(sizeof(int) * 4);
		underrun[-1] = 1;
		tklb_free(underrun);

		if (!heapCorruption) {
			return 2;
		}
		heapCorruption = false;
	}
	return 0;
}
