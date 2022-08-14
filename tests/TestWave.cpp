
#include "./TestCommon.hpp"

#include "../src/types/audio/resampler/TResamplerHold.hpp"
#include "../src/types/audio/resampler/TResamplerLinear.hpp"

#define TKLBZ_AUDIOFILE_IMPL
#include "../src/types/audio/TWaveFile.hpp"

using Resampler = tklb::ResamplerLinear;

int test() {
	{
		#ifdef TKLB_TEST
			return 0; // don't run the test in ci
		#endif
		const int blockSize = 256;
		const int rate2 = 50001;
		tklb::AudioBuffer sample;
		tklb::AudioBuffer resampled;
		tklb::wave::load<tklb::AudioBuffer::Sample, tklb::AudioBuffer>(
			"/home/usr/git/master/VAEG/VAE/external/tklb/tests/test_folder/LRMonoPhase4.wav", sample
		);
		Resampler resamplerUp(sample.sampleRate, rate2, blockSize);
		Resampler resamplerDown(rate2, sample.sampleRate, blockSize);
		resampled.resize(resamplerUp.calculateBufferSize(sample.size()), sample.channels());
		resampled.sampleRate = rate2;

		const int expectedUp = resamplerUp.estimateOut(sample.validSize());
		const int predictedDown = resamplerUp.estimateNeed(expectedUp);
		const int expectedDown = resamplerDown.estimateOut(expectedUp);
		const int predictedUp = resamplerDown.estimateNeed(expectedDown);
		resamplerUp.process(sample, resampled);
		// TKLB_ASSERT(expectedUp == resampled.validSize())
		resamplerDown.process(resampled, sample);
		// TKLB_ASSERT(expectedDown == sample.validSize())
		// const int sampleRate = 48000;
		// const int channels = 1;
		// const int length = sampleRate * 2;
		// sample.resize(length, channels);
		// sample.sampleRate = sampleRate;
		// sample.setValidSize(length);
		// for (int c = 0; c < channels; c++) {
		// 	for (int i = 0; i < length; i++) {
		// 		sample[c][i] = sin(i * 0.1);
		// 	}
		// }
		tklb::wave::write<tklb::AudioBuffer::Sample, tklb::AudioBuffer>(
			sample, "/home/usr/git/master/VAEG/VAE/external/tklb/tests/test_folder/test.wav"
		);
	}
	return 0;
}
