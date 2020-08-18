#include "../types/TRingBuffer.h"

template <typename T>
int test(T& buffer) {
	float in[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	buffer.add(in, 8);
	if (buffer.inBuffer() != 8) {
		return 2;
	}
	for (int i = 0; i < 10; i++) {
		buffer.add(in, 10);
	}
	if (buffer.nFree() != 0) {
		return 3;
	}
	if (buffer.inBuffer() != 100) {
		return 4;
	}
	float out[100];
	buffer.get(out, 100);
	if (buffer.inBuffer() != 0) {
		return 5;
	}
	if (out[8] != 0) {
		return 6;
	}
	return 0;
}

int main() {
	tklb::RingBuffer<float> buffer;
	tklb::RingBuffer<float> buffer2(100);
	if (buffer2.nFree() != 100) {
		return 1;
	}
	tklb::StackRingBuffer<float, 100> stackbuf;
	buffer.setSize(100);
	int ret = test(buffer);
	if (ret != 0) {
		return ret;
	}
	return test(stackbuf);

}