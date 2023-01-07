#include "./TestCommon.hpp"
#include "../src/types/THeapBuffer.hpp"
#include "../src/util/TTraits.hpp"
using namespace tklb;

struct Owner {
	int test = 0;
	HeapBuffer<char> buffer;
};

int test() {
	{
		auto sourceBuffer = HeapBuffer<char>();
		sourceBuffer.reserve(16);
		sourceBuffer.push('a');
		sourceBuffer.push('b');
		sourceBuffer.push('c');
		sourceBuffer.push('d');
		sourceBuffer.push('e');

		auto moved = traits::move(sourceBuffer);

		if (moved.injected()) {
			return 1;
		}
		if (!sourceBuffer.injected()) {
			return 2;
		}

		Owner owner;
		owner.buffer = traits::move(moved);

		if (!moved.injected()) {
			return 3;
		}

		if (owner.buffer.injected()) {
			return 4;
		}

		Owner ownerMoved;
		ownerMoved = traits::move(owner);

		ownerMoved.buffer[2] = 'z';

		(void) ownerMoved;
	}

	return 0;
}
