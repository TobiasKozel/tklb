#include "./TestCommon.hpp"
#include "../src/memory/TAllocator.hpp"

int test() {
	tklb::DefaultAllocator<> allocator;
	auto space = allocator.allocate(16);
	allocator.deallocate(space, 16);
	return 0;
}
