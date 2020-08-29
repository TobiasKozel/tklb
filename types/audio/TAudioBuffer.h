#ifndef TKLB_AUDIOBUFFER
#define TKLB_AUDIOBUFFER

#include <vector>
#include <cstring>

#ifndef TKLB_NO_INTRINSICS
	#include "../../external/xsimd/include/xsimd/xsimd.hpp"
#endif

#include "../../util/TAssert.h"

namespace tklb {

class AudioBuffer {

public:
	using uchar = unsigned char;
	using uint = unsigned int;
#ifdef TKLB_MAXCHANNELS
	static const uchar MAX_CHANNELS = TKLB_MAXCHANNELS;
#else
	static const uchar MAX_CHANNELS = 2;
#endif

#ifdef TKLB_SAMPLE_FLOAT
	using sample = float;
#else
	using sample = double;
#endif

private:
	using Buffer = std::vector<sample
#ifndef TKLB_NO_INTRINSICS
	, xsimd::aligned_allocator<sample, XSIMD_DEFAULT_ALIGNMENT>
#endif
	>;

	Buffer mBuffers[MAX_CHANNELS];
	uchar mChannels = 0;

public:
	AudioBuffer() = default;

	/**
	 * Set a single channel from a float array
	 * @param samples An Array containing the interleaved audio samples
	 * @param channel Channelindex
	 * @param length The length (single channel)
	 */
	void set(const float* samples, const uchar channel, const uint length) {
		TKLB_ASSERT(checkSize(length, channel))
		#ifdef TKLB_SAMPLE_FLOAT
			memcpy(mBuffers[channel].data(), samples, sizeof(float) * length);
		#else
			for (uint i = 0; i < length; i++) {
				mBuffers[channel][i] = samples[i];
			}
		#endif
	}

	/**
	 * Set a single channel from a double array
	 * @param samples An Array containing the interleaved audio samples
	 * @param channel Channelindex
	 * @param length The length (single channel)
	 */
	void set(const double* samples, const uchar channel, const uint length, const uint offset = 0) {
		TKLB_ASSERT(checkSize(length + offset, channel))
		#ifdef TKLB_SAMPLE_FLOAT
			for (uint i = 0; i < length; i++) {
				mBuffers[channel][i + offset] = samples[i];
			}
		#else
			memcpy(mBuffers[channel].data() + offset, samples, sizeof(double) * length);
		#endif
	}

	/**
	 * @param samples A 2D Array containing the audio samples (float or double)
	 * @param channels Channel count
	 * @param length The length (single channel)
	 */
	template <typename T>
	void set(const T** samples, uchar channels, uint length, const uint offset = 0) {
		for (uchar c = 0; c < length; c++) {
			set(samples[c], c, length, offset);
		}
	};

	/**
	 * @param samples A 1D Array containing the interleaved audio samples (float or double)
	 * @param channels Channel count
	 * @param length The length (single channel)
	 */
	template <typename T>
	void setFromInterleaved(const T* samples, uchar channels, uint length, const uint offset = 0) {
		TKLB_ASSERT(checkSize(length + offset, channels))
		for (uchar c = 0; c < channels; c++) {
			for(uint i = 0, j = c; i < length; i++, j+= channels) {
				mBuffers[c][i + offset] = samples[j];
			}
		}
	}

	/**
	 * Set from a buffer object
	 */
	void set(const AudioBuffer& buffer, const uint offset = 0) {
		TKLB_ASSERT(checkSize(buffer.getLength() + offset, buffer.getChannels()))
		for (uchar c = 0; c < buffer.getChannels(); c++) {
			memcpy(
				mBuffers[c].data() + offset, buffer.get(c),
				sizeof(sample) * buffer.getLength()
			);
		}
	};

	void resize(const uint length,  const uchar channels) {
		TKLB_ASSERT(channels <= MAX_CHANNELS)
		if (channels == mChannels && mBuffers[0].size() == length) {
			return;
		}
		for(uchar c = 0; c < channels; c++) {
			mBuffers[c].resize(length);
		}
		for (uchar c = channels; c < MAX_CHANNELS; c++) {
			mBuffers[c].resize(0);
		}
		mChannels = channels;
	}

	void resize(const AudioBuffer& buffer) {
		resize(buffer.getLength(), buffer.getLength());
	}

	void clear() {
		resize(0, 0);
	}

	void add(const AudioBuffer& buffer, uint offset = 0) {

	}

	uchar getChannels() const {
		return mChannels;
	}

	uint getLength() const {
		return mBuffers[0].size();
	}

	sample* get(const uchar channel) {
		return mBuffers[channel].data();
	};

	const sample* get(const uchar channel) const {
		return mBuffers[channel].data();
	};

private:
	bool checkSize(const uint length, uchar channels) {
		return mChannels >= channels && mBuffers[0].size() >= channels;
	}
};

}

#endif // TKLB_AUDIOBUFFER
