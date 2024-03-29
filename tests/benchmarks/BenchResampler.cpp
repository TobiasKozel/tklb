#include "../../src/types/audio/resampler/TResampler.hpp"
#include "./BenchmarkCommon.hpp"

int main() {
	{
		const int length = 530;
		const int channels = 16;
		const int rate1 = 44100;
		const int rate2 = 48000;
		Resampler up(rate1, rate2, length);
		Resampler down(rate2, rate1, length * 2); // Bigger max block obviously

		AudioBuffer in, out;
		in.sampleRate = rate1;
		out.sampleRate = rate2;

		in.resize(Resampler::calculateBufferSize(rate1, rate1, length), channels);
		out.resize(Resampler::calculateBufferSize(rate1, rate2, length) , channels); // same here

		// generate sine test signal
		for (int c = 0; c < channels; c++) {
			for (int i = 0; i < length; i++) {
				in[c][i] = sin(i * c * 0.001); // Failry low frequency
			}
		}
		in.setValidSize(length);

		{
			TIMER(Microseconds);
			for (int i = 0; i < ITERATIONS; i++) {
				up.process(in, out);
				down.process(out, in);
			}
		}

	}
	return 0;
}
