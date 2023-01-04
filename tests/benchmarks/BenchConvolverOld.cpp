#include "./BenchmarkCommon.hpp"
#include "../../src/types/audio/TAudioBuffer.hpp"
#include "../../src/types/audio/convolver/TConvolverOld.hpp"


int main() {
	{
		constexpr int channelCount = 16;
		Convolver con;
		const int audioLength = 520;
		int blockSize = 128;

		AudioBufferFloat ir, in, out;
		ir.resize(1024, channelCount);
		ir.set(0);
		for (int c = 0; c < channelCount; c++) {
			ir[c][1] = 1.0; // perfect impulse delaying the signal by one sample
			ir[c][512] = 1.0; // second delay to make the ir longer
		}
		con.load(ir, blockSize);

		in.resize(audioLength, channelCount);
		out.resize(in);

		for(int c = 0; c < channelCount; c++) {
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
