#define TKLB_MAXCHANNELS 16
#include "../types/audio/TOversampler.h"

#include "TestCommon.h"
#include <cmath>

int main() {
	const int length = 1024 * 1024;
	const int channels = TKLB_MAXCHANNELS;
	using uchar = unsigned char;

	{
		Oversampler<> oversampler;

		oversampler.setFactor(4);
		oversampler.setProcessFunc(
			[&](Oversampler<>::T** in, Oversampler<>::T** out, uint len) {
				for(uchar c = 0; c < channels; c++) {
					for (uint i = 0; i < len; i++) {
						out[c][i] = in[c][i] + 1;
					}
				}
		});

		AudioBuffer in, out;

		in.resize(length, channels);
		out.resize(in);

		for (uchar c = 0; c < channels; c++) {
			for (int i = 0; i < length; i++) {
				in[c][i] = sin(i * c * 0.1);
			}
		}

		oversampler.process(in, out);

		for (uchar c = 0; c < channels; c++) {
			for (int i = 0; i < length; i++) {
				if (!close(out[c][i], sin(i * c * 0.1)) + 1) {
					return 1;
				}
			}
		}
	}

	memcheck()
	return 0;
}
