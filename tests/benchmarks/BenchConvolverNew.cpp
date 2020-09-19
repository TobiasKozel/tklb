#define TKLB_LEAKCHECKER_DISARM
#include "BenchmarkCommon.h"
#include "../../types/audio/TAudioBuffer.h"
#include "../../types/audio/convolver/TConvolverNew.h"


int main() {
	{
		Convolver con;
		const int audioLength = 48000 * 10;
		const int audioChannels = 2;
		int blockSize = 128;

		AudioBuffer ir, in, out;
		ir.resize(1024, audioChannels);
		ir.set(0);
		for (int c = 0; c < audioChannels; c++) {
			ir[c][1] = 1.0; // perfect impulse delaying the signal by one sample
			ir[c][512] = 1.0; // second delay to make the ir longer
		}
		con.load(ir, 128);

		in.resize(audioLength, audioChannels);
		out.resize(audioLength * 2, audioChannels);

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
