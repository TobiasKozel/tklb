#include "../src/types/audio/TAudioBuffer.hpp"
// #include <cstdlib>

using namespace tklb;

int main() {
	volatile int size = 3;
	HeapBuffer<> test(size);
	return test.size();
}
