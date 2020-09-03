#include "../types/audio/fft/TOouraFFT.h"

#include "TestCommon.h"

int main() {
	{
		const int fftSize = 512;
		double imaginary[fftSize];
		double real[fftSize];
		FFT con = { 512 };
		AudioBuffer<> input, output;
		input.resize(fftSize, 1);
		output.resize(fftSize, 1);

		for (int i = 0; i < fftSize; i++) {
			input[0][i] = sin(i * 0.1);
		}

		con.forward(input.get(0), real, imaginary);
		con.back(output.get(0), real, imaginary);

		for (int i = 0; i < fftSize; i++) {
			if (!close(sin(i * 0.1), output[0][i])) {
				return 1;
			}
		}
	}

	memcheck()
	return 0;
}
