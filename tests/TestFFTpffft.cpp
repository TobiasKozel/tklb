#include "TestCommon.h"
#include "../types/audio/fft/TFFT.h"


int main() {
	{
		const int fftSize = 32;
		FFT con = { fftSize };
		AudioBuffer input, output, result;
		input.resize(fftSize, 1);
		output.resize(fftSize, 1);
		result.resize(fftSize, 2);

		for (int i = 0; i < fftSize; i++) {
			input[0][i] = sin(i * 0.1);
		}

		con.forward(input, result);
		con.back(result, output);

		for (int i = 0; i < fftSize; i++) {
			if (!close(sin(i * 0.1), output[0][i])) {
				return 1;
			}
		}
	}

	memcheck()
	return 0;
}
