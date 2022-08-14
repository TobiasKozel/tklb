#include "./TestCommon.hpp"

#include "../src/types/audio/TAudioBuffer.hpp"
#include "../src/types/audio/TAudioRingBuffer.hpp"

#include <cmath>

int test() {
	const int size = 1024;
	const int sourceSize = size * 10;
	const int channels = 4;
	tklb::AudioBuffer source, dest;
	source.resize(sourceSize, channels);
	dest.resize(sourceSize, channels);

	// generate sine test signal
	for (int c = 0; c < channels; c++) {
		for (int i = 0; i < sourceSize; i++) {
			source[c][i] = sin(i * (c + 5) * 0.001);
		}
	}

	{
		tklb::AudioRingBuffer buffer(size, channels);
		int inpos = 0;
			int outpos = 0;
		source.setValidSize(100);
		inpos += buffer.push(source, inpos);
		source.setValidSize(200);
		inpos += buffer.push(source, inpos);

		outpos += buffer.pop(dest, 50, 0, outpos);

		source.setValidSize(100);
		inpos += buffer.push(source, inpos);

		outpos += buffer.pop(dest, 350, 0, outpos);

		if (buffer.filled() != 0) {
			return 1;
		}

		if (buffer.remaining() != size) {
			return 2;
		}

	}

	// compare sine test signal
	for (int c = 0; c < channels; c++) {
		for (int i = 0; i < 400; i++) {
			if (!close(source[c][i], dest[c][i], 0.1)) {
				return 3;
			}
		}
	}

	return 0;
}
