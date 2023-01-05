#include "./TestCommon.hpp"
#include "../src/types/THandleBuffer.hpp"

struct ClassToStore {
	const char* name;
	int number;
};

int test()
{
	const unsigned int size = 4;
	tklb::HandleBuffer<ClassToStore> handleBuf(size);
	auto e0 = handleBuf.create();
	auto e1 = handleBuf.create();
	auto e2 = handleBuf.create();
	auto e3 = handleBuf.create();
	auto e4 = handleBuf.create();

	if (e4 != handleBuf.InvalidHandle) {
		return 1;
	}

	handleBuf[e1].number = 30;

	handleBuf.remove(e0);
	auto e6 = handleBuf.create();

	if (e6 == handleBuf.InvalidHandle) {
		return 2;
	}

	handleBuf.remove(e3);

	int count = 0;
	handleBuf.iterate([&](ClassToStore& element, unsigned int handle) {
		count++;
	});
	if (count != 3) {
		return 3;
	}

	count = 0;
	handleBuf.remove(e2);

	handleBuf.iterate([&](ClassToStore& element, unsigned int handle) {
		count++;
	});
	if (count != 2) {
		return 4;
	}

	bool removed = handleBuf.remove(e3);
	if (removed) {
		return 6;
	}
	removed = handleBuf.remove(e6);
	if (!removed) {
		return 7;
	}

	if (handleBuf[e1].number != 30) {
		return 8;
	}

	handleBuf.remove(e1);

	if (handleBuf.has(e1)) {
		return 9;
	}
	if (handleBuf.free() != size) {
		return 10;
	}
	return 0;
}
