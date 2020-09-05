#include "TestCommon.h"
#include "../types/audio/TResampler.h"


int main() {
	{
		Resampler<2> resampler(48000, 44100);
		float sample[1024] = { 0 };
		float* samples[2] = { sample, sample + 512 };
		int out = resampler.process(samples, 512);
		if (out != 471) {
			return 1;
		}
	}
	memcheck()
	return 0;
}
