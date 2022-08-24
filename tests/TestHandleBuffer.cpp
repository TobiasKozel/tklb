#include "./TestCommon.hpp"
#include "../src/types/THandleBuffer.hpp"

struct ClassToStore {
	const char* name;
	int number;
};

int test()
{
	int size = 4;
	tklb::HandleBuffer<ClassToStore> handleBuf(size);
	auto e0 = handleBuf.create();
	auto e1 = handleBuf.create();
	auto e2 = handleBuf.create();
	auto e3 = handleBuf.create();
	auto e4 = handleBuf.create();

	if (e4 != handleBuf.InvalidHandle) {
		return 1;
	}


	handleBuf.remove(e0);
	auto e6 = handleBuf.create();

	handleBuf.remove(e1);
	handleBuf.remove(e2);
	handleBuf.remove(e3);
	handleBuf.remove(e6);

	if (handleBuf.free() != size) {
		return 2;
	}
	return 0;
}
