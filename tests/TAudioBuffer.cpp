#define TKLB_NO_INTRINSICS

#include "../types/audio/TAudioBuffer.h"
#include "../util/TLeakChecker.h"
#include "../util/TTimer.h"

int ret;
#define returnNonZero(val) ret = val; if(ret != 0) { return ret; }

const int size = 1024;
const int channels = 2;

bool close(float a, float b) {
	return std::abs(a - b) < 0.01;
}

int deinterleave(tklb::AudioBuffer& buffer) {
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

	if (buffer.size() != size) {
		return 3;
	}

	if (buffer.channels() != channels) {
		return 4;
	}
	return 0;
}

template <typename T>
int conversion(tklb::AudioBuffer& buffer) {
	buffer.resize(0, 0);
	buffer.resize(size, channels);
	T fsamples[channels * size];
	std::fill_n(fsamples, channels * size, 1.0);
	T* fbuf[channels] = { };

	for (int c = 0; c < channels; c++) {
		fbuf[c] = fsamples + (size * c);
	}

	buffer.set(fbuf, channels, size, size / 2);

	auto l = buffer.get(0);
	auto r = buffer.get(1);

	for (int i = 0; i < size; i++) {
		tklb::AudioBuffer::sample expected = i >= size / 2 ? 1.0 : 0;
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
	tklb::AudioBuffer buffer, buffer2;
	buffer.resize(size, channels);
	buffer2.resize(size, channels);

	tklb::AudioBuffer::sample fsamples[channels * size];
	std::fill_n(fsamples, channels * size, 1.0);
	tklb::AudioBuffer::sample* fbuf[channels] = { };

	for (int c = 0; c < channels; c++) {
		fbuf[c] = fsamples + (size * c);
	}

	buffer.set(fbuf, channels, size, size / 2);

	std::fill_n(fsamples, channels * size, 0.0);
	for (int c = 0; c < channels; c++) {
		for (int i = 0; i < size / 2; i++) {
			fbuf[c][i] = 1.0;
		}
	}

	buffer2.set(fbuf, channels, size);

	{
		tklb::SectionClock timer("Simd add took ");
		for(int i = 0; i < 10000; i++) {
			buffer.add(buffer2);
		}
	}

	return 0;
}

int main() {

	{
		tklb::AudioBuffer buffer;

		returnNonZero(deinterleave(buffer))

		returnNonZero(conversion<float>(buffer))

		returnNonZero(conversion<double>(buffer))

		returnNonZero(add())

	}

	if (tklb::allocationCount != 0) {
		return 7;
	}

	if (tklb::curruptions != 0) {
		return 8;
	}

	return 0;
}
