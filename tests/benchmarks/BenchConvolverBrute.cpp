#define TKLB_LEAKCHECKER_DISARM
#include "BenchmarkCommon.h"
#include "../../types/audio/TAudioBuffer.h"
#include "../../types/audio/convolver/TConvolverBrute.h"


int main() {
	{
		ConvolverBrute con;
		const int audioLength = 1024 * 100;
		const int audioChannels = 2;
		int blockSize = 128;

		AudioBufferFloat ir, in, out;
		ir.resize(1024, audioChannels);
		ir.set(0);
		for (int c = 0; c < audioChannels; c++) {
			ir[c][1] = 1.0; // perfect impulse delaying the signal by one sample
			ir[c][512] = 1.0; // second delay to make the ir longer
		}
		con.load(ir, 128);

		in.resize(audioLength, audioChannels);
		out.resize(in);

		for(int c = 0; c < audioChannels; c++) {
			for(int i = 0; i < audioLength; i++) {
				in[c][i] = (i + c) % 2;
			}
		}

		{
			TIMER(Microseconds);
			con.process(in, out);
		}

	}
	return 0;
}
