#define TKLB_NO_ASSERT
#include "../src/memory/TFixedPool.hpp"

int main() {
	constexpr int PoolSize = 1024;
	constexpr int chunkSize = 7;
	char* memory = new char[PoolSize];
	{
		tklb::memory::FixedPool pool(memory, PoolSize);
		int allocated = 0;
		while (true) {
			void* mem = pool.allocate(chunkSize);
			if (mem == nullptr) { break; }
			allocated += pool.realAllocation(chunkSize);
			if (PoolSize < allocated ) {
				return 1;
			}
		}
		delete[] memory;
		if (allocated == pool.allocated()) {
			return 0;
		}
		return  2;
	}
}
