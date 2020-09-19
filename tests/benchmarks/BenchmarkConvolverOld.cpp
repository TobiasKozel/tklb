#define TKLB_LEAKCHECKER_DISARM
#include "BenchmarkCommon.h"
#include "../../types/audio/convolver/TConvolverOld.h"


int main() {
	{
		Convolver<> con;
		const int irLength = 1024;
		const int audioLength = 48000 * 10; // ten seconds of audio
		const int audioChannels = 2;
		const int blockSize = 128;

		float irSamples[irLength];
		for (int i = 0; i < irLength; i++) {
			irSamples[i] = sin(i * 0.01);
		}
		float* ir[1] = { irSamples }; // one channel for the IR
		con.loadIR(ir, irLength, 1);


		float inSamples[audioChannels * audioLength];
		float* in[audioChannels] = { inSamples +  0, inSamples + audioLength };

		for(int c = 0; c < audioChannels; c++) {
			for(int i = 0; i < audioLength; i++) {
				inSamples[i + (c * audioLength)] = (i + c) % 2;
			}
		}

		float outSamples[audioChannels * audioLength];
		float* out[audioChannels] = { outSamples +  0, outSamples + audioLength };

		{
			TIMER(Microseconds);
			for(int b = 0; b < audioLength; b += blockSize) {
				for(int c = 0; c < audioChannels; c++) {
					con.process(in, out, blockSize);
					in[c] += blockSize;
					out[c] += blockSize;
				}
			}
		}

	}
	return 0;
}
