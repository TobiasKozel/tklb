#define TKLB_MAXCHANNELS 16
#include "../../types/audio/fft/Tpffft.h"
#include "BenchmarkCommon.h"

int main() {
	{
		const int fftSize = 512;
		FFT con = { fftSize };
		AudioBuffer<> input, output;
		AudioBuffer<float> result;
		input.resize(fftSize, 1);
		output.resize(fftSize, 1);
		result.resize(fftSize / 2, 2);

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