#include "./TestCommon.hpp"
#include "../src/types/audio/fft/TFFT.hpp"

int test() {
	const int blocks = 100;
	const int fftSize = 128;
	const int bufferLength = fftSize * blocks;
	tklb::FFT con = { fftSize };
	tklb::AudioBuffer input, output, result;
	input.resize(bufferLength);
	result.resize(con.resultSize(bufferLength), 2);
	output.resize(bufferLength);

	for (int i = 0; i < bufferLength; i++) {
		input[0][i] = tklb::sin(i * 0.1);
	}

	con.forward(input, result);
	con.back(result, output);

	for (int i = 0; i < bufferLength; i++) {
		if (!close(input[0][i], output[0][i])) {
			return 1;
		}
	}

	return 0;
}
