#include "./TestCommon.hpp"
#include "../src/types/THeapBuffer.hpp"


int test() {
	tklb::HeapBuffer<float*> list;
	const int size = 1024;
	float arr[size];

	for (int i = 0; i < size; i++) {
		arr[i] = i;
		list.push(arr + i);
	}

	if (list.size() != size) {
		return 1;
	}

	for (int i = 0; i < size; i++) {
		if (*list[i] != i) {
			return 2;
		}
	}

	for (int i = 0; i < size - 1; i++) {
		list.remove(arr + i);
	}

	if (*list[0] != size - 1) {
		return 3;
	}

	if (list.size() != 1) {
		return 4;
	}

	list.clear();

	if (list.size() != 0) {
		return 5;
	}

	return 0;
}
