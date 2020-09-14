#include "TestCommon.h"
#include "../types/audio/TConvolverNew.h"


int main() {
	{
		SingleConvolver con;
		// float irSamples[1024] = { 0.0 };
		// irSamples[1] = 1.0; // perfect impulse delaying the signal by one sample
		// float* ir[1] = { irSamples };
		// con.loadIR(ir, 1024, 1);

		// const int audioLength = 1024 * 10;
		// const int audioChannels = 2;

		// float inSamples[audioChannels * audioLength];
		// float* in[audioChannels] = { inSamples +  0, inSamples + audioLength };

		// for(int c = 0; c < audioChannels; c++) {
		// 	for(int i = 0; i < audioLength; i++) {
		// 		inSamples[i + (c * audioLength)] = (i + c) % 2;
		// 	}
		// }

		// float outSamples[audioChannels * audioLength];
		// float* out[audioChannels] = { outSamples +  0, outSamples + audioLength };

		// int blockSize = 128;
		// for(int b = 0; b < audioLength; b += blockSize) {
		// 	for(int c = 0; c < audioChannels; c++) {
		// 		con.process(in, out, blockSize);
		// 		in[c] += blockSize;
		// 		out[c] += blockSize;
		// 	}
		// }

		// /**
		//  * Skip the first and last sample due to the delay
		//  */
		// for(int i = 1; i < (audioLength - 1) * audioChannels; i++) {
		// 	if (!close(inSamples[i], outSamples[i + 1])) {
		// 		return 1;
		// 	}
		// }
	}

	memcheck()
	return 0;
}
