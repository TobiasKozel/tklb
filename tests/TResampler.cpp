#include "../types/audio/TResampler.h"
#define TKLB_LEAKCHECKER_NO_INFO
#include "../util/TLeakChecker.h"

int main() {
	{
		tklb::Resampler<2> resampler(48000, 44100);
		float sample[1024] = { 0 };
		float* samples[2] = { sample, sample + 512 };
		int out = resampler.process(samples, 512);
		if (out != 471) {
			return 1;
		}
	}
	if (tklb::allocationCount != 0) {
		return 2;
	}
	{
		tklb::HeapResampler<> resampler(48000, 44100, 2);
		float sample[1024] = { 0 };
		float* samples[2] = { sample, sample + 512 };
		int out = resampler.process(samples, 512);
		if (out != 471) {
			return 3;
		}
	}
	if (tklb::allocationCount != 0) {
		return 4;
	}
	return 0;
}
