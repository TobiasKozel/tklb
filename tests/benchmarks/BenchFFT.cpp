#define TKLB_MAXCHANNELS 16
#include "../../types/audio/fft/TOouraFFT.h"
#include "BenchmarkCommon.h"

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

		{
			TIMER(Miliseconds);
			for (int i = 0; i < 10000; i++) {
				con.forward(input.get(0), real, imaginary);
				con.back(output.get(0), real, imaginary);
			}
		}
	}

	return 0;
}
