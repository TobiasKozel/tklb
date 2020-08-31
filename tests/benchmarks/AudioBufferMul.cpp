#define TKLB_MAXCHANNELS 16
#include "../../types/audio/TAudioBuffer.h"
#include "BenchmarkCommon.h"

int main() {
	// some overhang so simd can't do all of it
	const int length = 130;
	const int channels = TKLB_MAXCHANNELS;
	const int bufferCount = 2;
	AudioBuffer buffers[bufferCount];

	for (int i = 0; i < bufferCount; i++) {
		buffers[i].resize(length, channels);
		for (int c = 0; c < channels; c++) {
			auto channel = buffers[i].get(i);
			fill_n(channel, length, i);
		}
	}

	{
		TIMER(Miliseconds);
		for(int i = 0; i < 100000; i++) {
			buffers[0].multiply(buffers[1]);
		}
	}
	return 0;
}
