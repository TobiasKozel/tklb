#include "./TestCommon.hpp"
#include "../src/types/audio/TAudioBuffer.hpp"



const int length = 1024;
const int channels = 2;

int deinterleave(tklb::AudioBuffer& buffer) {
	buffer.resize(length, channels);

	float noconvInterleaved[length * channels];

	for (int i = 0; i < length * channels; i += channels) {
		noconvInterleaved[i] = 1.0;
		noconvInterleaved[i + 1] = 0.0;
	}

	buffer.setFromInterleaved(noconvInterleaved, length, channels);

	auto deinterleavedL = buffer[0];
	auto deinterleavedR = buffer[1];

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
int conversion(tklb::AudioBuffer& buffer) {
	buffer.resize(0);
	buffer.resize(length, channels);
	buffer.set(0); // important
	T fsamples[channels * length];
	for (int i = 0; i < channels * length; i++) {
		fsamples[i] = 1.0; // only set part of it to one
	}
	T* fbuf[channels] = { };

	for (int c = 0; c < channels; c++) {
		fbuf[c] = fsamples + (length * c);
	}

	buffer.set(fbuf, length, channels, 0, length / 2);

	tklb::AudioBuffer::Sample* l = buffer[0];
	tklb::AudioBuffer::Sample* r = buffer[1];

	for (int i = 0; i < length; i++) {
		tklb::AudioBuffer::Sample expected = (i >= (length / 2)) ? 1.0 : 0;
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
	tklb::AudioBuffer buffer1, buffer2, buffer3;

	buffer1.resize(length, channels);
	buffer1.set(1.0, length / 2);
	buffer1.set(0.0, length / 2, length / 2);

	buffer2.resize(buffer1);
	buffer2.set(0.0, length / 2);
	buffer2.set(1.0, length / 2, length / 2);

	buffer3.resize(buffer1);
	buffer3.set(1.0);
	buffer3.multiply(-1);

	buffer1.add(buffer2); // should be all 1.0 now
	buffer1.add(buffer3); // should be all 0.0 now

	/**
	 * Check if the whole buffer is 1.0 now
	 */
	for (int c = 0; c < channels; c++) {
		for (int i = 0; i < length; i++) {
			if (!close(buffer1[c][i], 0.0)) {
				return 7;
			}
		}
	}
	return 0;
}

int casts() {
	tklb::AudioBufferDouble d1, d2;
	tklb::AudioBufferFloat f1, f2;
	d1.resize(length, channels);
	d2.resize(d1);
	f1.resize(d1);
	f2.resize(d1);

	d1.add(d2);
	d1.set(d2);
	d1.multiply(d2);
	d2.add(f2);
	d2.set(f2);
	d2.multiply(f2);

	f1.add(f2);
	f1.set(f2);
	f1.multiply(f2);
	f1.add(d2);
	f1.set(d2);
	f1.multiply(d2);
	return 0;
}

int test() {
	tklb::AudioBuffer buffer;

	returnNonZero(deinterleave(buffer))
	returnNonZero(conversion<float>(buffer))
	returnNonZero(conversion<double>(buffer))

	returnNonZero(add())

	returnNonZero(casts())
	return 0;
}
