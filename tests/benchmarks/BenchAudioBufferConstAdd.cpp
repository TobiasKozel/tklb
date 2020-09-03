#define TKLB_MAXCHANNELS 16
#include "../../types/audio/TAudioBuffer.h"
#include "BenchmarkCommon.h"

int main() {
	// some overhang so simd can't do all of it
	const int length = 495;
	const int channels = TKLB_MAXCHANNELS;
	AudioBuffer<> buffer;

	buffer.resize(length, channels);
	for (int c = 0; c < channels; c++) {
		auto channel = buffer.get(c);
		fill_n(channel, length, 2);
	}

	{
		TIMER(Miliseconds);
		for(int i = 0; i < 100000; i++) {
			buffer.add(10.0);
		}
	}
	return 0;
}
