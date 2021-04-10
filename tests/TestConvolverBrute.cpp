#include "./TestCommon.hpp"
#include "../src/types/audio/convolver/TConvolverBrute.hpp"


int test() {
	const int audioLength = 48000 * 10;
	const int audioChannels = 2;
	const int blockSize = 128;

	using T = AudioBuffer::sample;
	ConvolverBrute con;
	AudioBuffer ir, in, out;
	ir.resize(1024, audioChannels);
	ir.set(0);
	for (int c = 0; c < audioChannels; c++) {
		// perfect impulse delaying the signal by 1 + channel index
		ir[c][1 + c] = 1.0;
	}
	con.load(ir, blockSize);

	in.resize(audioLength, 1); // Mono input
	out.resize(audioLength, audioChannels);

	for(int i = 0; i < audioLength; i++) {
		in[0][i] = i % 2;
	}

	con.process(in, out);

	for (char c = 0; c < audioChannels; c++) {
		for(int i = 0; i < (audioLength - audioChannels); i++) {
			if (!close(in[0][i], out[c][i + 1 + c])) {
				return 1;
			}
		}
	}
	return 0;
}
