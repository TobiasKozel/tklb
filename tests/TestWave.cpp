#include "./TestCommon.hpp"

#include "../src/types/audio/resampler/TResamplerHold.hpp"
#include "../src/types/audio/resampler/TResamplerLinear.hpp"
#include <cstdio>
#include <cstdlib>

#include "../src/types/audio/TWave.hpp"

using Resampler = tklb::ResamplerLinear;

int test() {
	{
		const char* filePath = "./test_folder/ir.wav";
		auto file = fopen(filePath, "r");

		if (!file) { return 1; }
		fseek(file, 0, SEEK_END);
		auto fileSize = ftell(file);
		if (!fileSize) {
			return 2;
		}
		char* fileContent = (char*) tklb_malloc(fileSize);
		fseek(file, 0, SEEK_SET);
		const auto read = fread(fileContent, fileSize, 1, file);
		fclose(file);
		file = 0;

		const int blockSize = 256;
		const int rate2 = 50001;
		tklb::AudioBuffer sample;
		tklb::AudioBuffer resampled;

		tklb::wave::load<tklb::AudioBuffer::Sample>(fileContent, fileSize, sample);

		tklb_free(fileContent);

		Resampler resamplerUp(sample.sampleRate, rate2, blockSize);
		Resampler resamplerDown(rate2, sample.sampleRate, blockSize);
		resampled.resize(resamplerUp.calculateBufferSize(sample.size()), sample.channels());
		resampled.sampleRate = rate2;

		// const int expectedUp = resamplerUp.estimateOut(sample.validSize());
		// const int predictedDown = resamplerUp.estimateNeed(expectedUp);
		// const int expectedDown = resamplerDown.estimateOut(expectedUp);
		// const int predictedUp = resamplerDown.estimateNeed(expectedDown);
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

		tklb::HeapBuffer<char> waveData;
		const char* outPath = "./test_folder/test.wav";
		tklb::wave::write<tklb::AudioBuffer::Sample, tklb::AudioBuffer>(sample, waveData);
		file = fopen(outPath, "w");
		if (!file) {
			return 10;
		}
		fwrite(waveData.data(), waveData.size(), 1, file);
		fclose(file);
	}
	return 0;
}
