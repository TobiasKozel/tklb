#include "../types/audio/TAudioBuffer.h"

#include "./TestCommon.h"

const int length = 1024;
const int channels = 2;

int deinterleave(AudioBuffer& buffer) {
	buffer.resize(length, channels);

	float noconvInterleaved[length * channels];

	for (int i = 0; i < length * channels; i += channels) {
		noconvInterleaved[i] = 1.0;
		noconvInterleaved[i + 1] = 0.0;
	}

	buffer.setFromInterleaved(noconvInterleaved, channels, length);

	auto deinterleavedL = buffer.get(0);
	auto deinterleavedR = buffer.get(1);

	for (int i = 0; i < length; i++) {
		if (!close(deinterleavedL[i], 1.0)) {
			return 1;
		}
		if (!close(deinterleavedR[i], 0.0)) {
			return 2;
		}
	}

	if (buffer.size() != length) {
		return 3;
	}

	if (buffer.channels() != channels) {
		return 4;
	}
	return 0;
}

template <typename T>
int conversion(AudioBuffer& buffer) {
	buffer.resize(0, 0);
	buffer.resize(length, channels);
	T fsamples[channels * length];
	std::fill_n(fsamples, channels * length, 1.0);
	T* fbuf[channels] = { };

	for (int c = 0; c < channels; c++) {
		fbuf[c] = fsamples + (length * c);
	}

	buffer.set(fbuf, channels, length, length / 2);

	auto l = buffer.get(0);
	auto r = buffer.get(1);

	for (int i = 0; i < length; i++) {
		AudioBuffer::T expected = i >= length / 2 ? 1.0 : 0;
		if (!close(expected, l[i])) {
			return 5;
		}
		if (!close(expected, r[i])) {
			return 6;
		}
	}

	return 0;
}

int add() {
	AudioBuffer buffer, buffer2;
	buffer.resize(length, channels);
	buffer2.resize(length, channels);

	AudioBuffer::T fsamples[channels * length];
	AudioBuffer::T* fbuf[channels] = { };

	/**
	 * Set the last half to 1.0 of the first buffer
	 */
	fill_n(fsamples, channels * length, 1.0);
	for (int c = 0; c < channels; c++) {
		fbuf[c] = fsamples + (length * c);
	}
	buffer.set(fbuf, channels, length, length / 2);

	/**
	 * Fill the first half with 1.0 of the second buffer
	 */
	fill_n(fsamples, channels * length, 0.0);
	for (int c = 0; c < channels; c++) {
		for (int i = 0; i < length / 2; i++) {
			fbuf[c][i] = 1.0;
		}
	}
	buffer2.set(fbuf, channels, length);

	/**
	 * Add them together
	 */
	buffer.add(buffer2);

	auto out = buffer.get(0);

	/**
	 * Check if the whole buffer is 1.0 now
	 */
	for (int i = 0; i < length; i++) {
		if (!close(out[i], 1.0)) {
			return 7;
		}
	}
	return 0;
}

int main() {

	{
		AudioBuffer buffer;

		returnNonZero(deinterleave(buffer))
		returnNonZero(conversion<float>(buffer))
		returnNonZero(conversion<double>(buffer))

		returnNonZero(add())
	}

	memcheck()
	return 0;
}
