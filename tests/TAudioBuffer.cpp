#include "../types/audio/TAudioBuffer.h"

bool close(float a, float b) {
	return std::abs(a - b) < 0.01;
}

int main() {
	tklb::AudioBuffer buffer;
	const int size = 1024;
	const int channels = 2;

	buffer.resize(size, channels);

	float noconvInterleaved[size * channels];

	for (int i = 0; i < size * channels; i += channels) {
		noconvInterleaved[i] = 1.0;
		noconvInterleaved[i + 1] = 0.0;
	}

	buffer.setFromInterleaved(noconvInterleaved, channels, size);

	auto deinterleavedL = buffer.get(0);
	auto deinterleavedR = buffer.get(1);

	for (int i = 0; i < size; i++) {
		if (!close(deinterleavedL[i], 1.0)) {
			return 1;
		}
		if (!close(deinterleavedR[i], 0.0)) {
			return 2;
		}
	}

	if (buffer.getLength() != size) {
		return 3;
	}

	if (buffer.getChannels() != channels) {
		return 4;
	}



	return 0;
}
