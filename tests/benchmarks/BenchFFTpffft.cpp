#include "../../src/types/audio/fft/TFFT.hpp"

#include "./BenchmarkCommon.hpp"

int main() {
	{
		const int fftSize = 512;
		FFT con = { fftSize };
		AudioBuffer input, output, result;
		input.resize(fftSize, 1);
		output.resize(fftSize, 1);
		result.resize(fftSize / 2, 2);

		for (int i = 0; i < fftSize; i++) {
			input[0][i] = sin(i * 0.1);
		}

		{
			TIMER(Microseconds);
			for (int i = 0; i < ITERATIONS * 16; i++) {
				con.forward(input, result);
				con.back(result, output);
			}
		}
	}

	return 0;
}
