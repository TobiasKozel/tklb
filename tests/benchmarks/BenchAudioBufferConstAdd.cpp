#include "../../src/types/audio/TAudioBuffer.hpp"

#define ITERATIONS 10000
#include "./BenchmarkCommon.hpp"

int main() {
	// some overhang so simd can't do all of it
	const int length = 530;
	const int channels = 16;
	AudioBuffer buffer;

	buffer.resize(length, channels);
	for (int c = 0; c < channels; c++) {
		auto channel = buffer.get(c);
		fill_n(channel, length, 2);
	}

	{
		TIMER(Microseconds);
		for(int i = 0; i < ITERATIONS; i++) {
			buffer.add(10.0);
		}
	}
	return 0;
}
