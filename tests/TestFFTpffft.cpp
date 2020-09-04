#include "../types/audio/fft/Tpffft.h"

#include "TestCommon.h"

int main() {
	{
		const int fftSize = 32;
		AudioBuffer<float> result;
		FFT con = { fftSize };
		AudioBuffer<> input, output;
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