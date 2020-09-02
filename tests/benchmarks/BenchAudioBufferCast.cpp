#define TKLB_MAXCHANNELS 16
#include "../../types/audio/TAudioBuffer.h"
#include "BenchmarkCommon.h"

int main() {
	// some overhang so simd can't do all of it
	int length = 490;
	const int channels = TKLB_MAXCHANNELS;
	AudioBuffer abuffer;
	abuffer.resize(length, channels);

	#ifdef TKLB_SAMPLE_FLOAT
		using sample = double;
	#else
		using sample = float;
	#endif

	sample sbuf[length * channels];
	sample* buf[channels];

	for (int c = 0; c < channels; c++) {
		buf[c] = sbuf + length * c;
	}

	{
		TIMER(Miliseconds);
		for(int i = 0; i < 100000; i++) {
			abuffer.set(buf, channels, length);
		}
	}
	return 0;
}
