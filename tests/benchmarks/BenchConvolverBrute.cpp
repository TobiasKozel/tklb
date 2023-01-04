#define ITERATIONS 20
#include "./BenchmarkCommon.hpp"
#include "../../src/types/audio/TAudioBuffer.hpp"
#include "../../src/types/audio/convolver/TConvolverBrute.hpp"


int main() {
	{
		constexpr int maxChannels = 16;
		ConvolverBrute con;
		const int audioLength = 520;
		const int blockSize = 128;

		AudioBufferFloat ir, in, out;
		ir.resize(1024, maxChannels);
		ir.set(0);
		for (int c = 0; c < maxChannels; c++) {
			ir[c][1] = 1.0; // perfect impulse delaying the signal by one sample
			ir[c][512] = 1.0; // second delay to make the ir longer
		}
		con.load(ir, blockSize);

		in.resize(audioLength, maxChannels);
		out.resize(in);

		for(int c = 0; c < maxChannels; c++) {
			for(int i = 0; i < audioLength; i++) {
				in[c][i] = (i + c) % 2;
			}
		}

		{
			TIMER(Microseconds);
			for(int i = 0; i < ITERATIONS; i++) {
				con.process(in, out);
			}
		}

	}
	return 0;
}
