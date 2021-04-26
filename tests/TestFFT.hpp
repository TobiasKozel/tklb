#include "./TestCommon.hpp"
#include "../src/types/audio/fft/TFFT.hpp"

int test() {
	const int fftSize = 128;
	const int bufferLength = fftSize * 100;
	FFT con = { fftSize };
	AudioBuffer input, output, result;
	input.resize(bufferLength);
	output.resize(bufferLength);
	result.resize(bufferLength, 2);

	for (int i = 0; i < bufferLength; i++) {
		input[0][i] = sin(i * 0.1);
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