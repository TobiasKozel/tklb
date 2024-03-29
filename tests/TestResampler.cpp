
#include "./TestCommon.hpp"

#include "../src/types/audio/resampler/TResamplerHold.hpp"
#include "../src/types/audio/resampler/TResamplerLinear.hpp"



template <class Resampler>
int doTest() {
	const int rateLow = 44100;
	const int rateHigh = 48000;

	const int length = 4096;
	const int blockSize = 256;
	const int channels = 16;

	Resampler resamplerUp;
	resamplerUp.init(rateLow, rateHigh, blockSize, channels);
	Resampler resamplerDown;
	resamplerDown.init(rateHigh, rateLow, blockSize, channels);

	// TODO this isn't realy accurate since the delay is in the current samplerate or the resampler
	int latency = resamplerDown.getLatency();
	latency += resamplerUp.getLatency();

	tklb::AudioBuffer bufLow, bufHigh, bufLowReference;
	bufLow.sampleRate = rateLow;
	bufHigh.sampleRate = rateHigh;

	bufLow.resize(length + 10, channels); // add some padding
	bufHigh.resize(resamplerUp.calculateBufferSize(length) , channels);

	// generate sine test signal
	for (int c = 0; c < channels; c++) {
		for (int i = 0; i < length; i++) {
			bufLow[c][i] = tklb::sin(i * c * 0.001); // Fairly low frequency
		}
	}
	bufLow.setValidSize(length); // all samples in the buffer are to be processed
	bufLowReference.clone(bufLow); // Copy to compare

	// const int expectedUp = resamplerUp.estimateOut(bufLow.validSize());
	// const int predictedDown = resamplerUp.estimateNeed(expectedUp);
	// const int expectedDown = resamplerDown.estimateOut(expectedUp);
	// const int predictedUp = resamplerDown.estimateNeed(expectedDown);
	resamplerUp.process(bufLow, bufHigh);
	// TKLB_ASSERT(expectedUp == bufHigh.validSize())
	resamplerDown.process(bufHigh, bufLow);
	// TKLB_ASSERT(expectedDown == bufLow.validSize())

	// compare sine test signal
	for (int c = 0; c < channels; c++) {
		for (int i = 10; i < length - latency - 10; i++) { // crop the ends and latency
			if (!close(bufLow[c][i + latency], bufLowReference[c][i], 0.1)) {
				return 1;
			}
		}
	}
	return 0;
}

int test() {
	if (doTest<tklb::ResamplerHold>() != 0) {
		return 1;
	}
	if (doTest<tklb::ResamplerLinear>() != 0) {
		return 2;
	}
	return 0;
}
