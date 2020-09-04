#define TKLB_MAXCHANNELS 16
#include "../../types/audio/fft/TOouraFFT.h"
#include "BenchmarkCommon.h"

int main() {
	{
		const int fftSize = 512;
		FFT con = { fftSize };
		AudioBuffer<> input, output;
		AudioBuffer<double> result; // ooura only does doubles
		input.resize(fftSize, 1);
		output.resize(fftSize, 1);

		for (int i = 0; i < fftSize; i++) {
			input[0][i] = sin(i * 0.1);
		}

		{
			TIMER(Miliseconds);
			for (int i = 0; i < 10000; i++) {
				con.forward(input, result);
				con.back(result, output);
			}
		}
	}

	return 0;
}
