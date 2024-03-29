#include "../../src/types/audio/TOversampler.hpp"

#include "./BenchmarkCommon.hpp"
#include <cmath>

int main() {
	const int length = 490; // must be less than the max block size
	const int channels = 16;
	using uchar = unsigned char;
	using uint = unsigned int;

	{
		Oversampler<> oversampler;

		oversampler.setFactor(4);
		oversampler.setProcessFunc(
			[&](Oversampler<>::T** in, Oversampler<>::T** out, uint len) {
				for(uchar c = 0; c < channels; c++) {
					// memcpy(out[c], in[c], sizeof(Oversampler<>::T) * len);
				}
		});

		AudioBuffer in, out;

		in.resize(length, channels);
		out.resize(in);

		for (uchar c = 0; c < channels; c++) {
			for (int i = 0; i < length; i++) {
				in[c][i] = sin(i * c * 0.001); // Failry low frequency
			}
		}

		{
			TIMER(Microseconds);
			for (int i = 0; i < ITERATIONS; i++) {
				oversampler.process(in, out);
			}
		}
		int i = 0;

	}

	return 0;
}
