#include "../src/memory/TFixedPool.hpp"
#include <cstdlib>

int main() {
	constexpr int PoolSize = 1024;
	constexpr int chunkSize = 7;
	void* memory = malloc(PoolSize);
	{
		tklb::memory::FixedPool pool(memory, PoolSize);
		int allocated = 0;
		while (true) {
			void* mem = pool.allocate(chunkSize);
			if (mem == nullptr) { break; }
			allocated += pool.realAllocation(chunkSize);
		}
		free(memory);

		if (allocated == pool.allocated()) {
			return 0;
		}
		return  1;
	}
}
