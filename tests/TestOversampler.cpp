#include "TestCommon.h"
#define TKLB_MAXCHANNELS 16
#include "../types/audio/TOversampler.h"

#include <cmath>

int main() {
	const int length = 512; // default max block
	const int channels = TKLB_MAXCHANNELS;
	using uchar = unsigned char;
	using uint = unsigned int;


	{
		Oversampler<> oversampler;

		oversampler.setFactor(4);
		oversampler.setProcessFunc(
			[&](Oversampler<>::T** in, Oversampler<>::T** out, uint len) {
				for(uchar c = 0; c < channels; c++) {
					for (uint i = 0; i < len; i++) {
						out[c][i] = -in[c][i];
					}
				}
		});

		AudioBuffer<> in, out;

		in.resize(length, channels);
		out.resize(in);

		for (uchar c = 0; c < channels; c++) {
			for (int i = 0; i < length; i++) {
				in[c][i] = sin(i * c * 0.001); // Failry low frequency
			}
		}

		oversampler.process(in, out);

		for (uchar c = 0; c < channels; c++) {
			for (int i = 10; i < length - 10; i++) {
				if (!close(out[c][i], -sin(i * c * 0.001), 0.1)) {
					return 1;
				}
			}
		}
	}

	memcheck()
	return 0;
}
