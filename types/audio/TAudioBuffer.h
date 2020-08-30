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
	static constexpr uchar MAX_CHANNELS = TKLB_MAXCHANNELS;
#else
	static constexpr uchar MAX_CHANNELS = 2;
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
	 * @param channel Channel index
	 * @param length The length
	 */
	void set(const float* samples, const uchar channel, uint length, uint offset = 0) {
		if (mChannels <= channel) { return; }
		length = std::min(length - offset, size() - offset);
		#ifdef TKLB_SAMPLE_FLOAT
			memcpy(mBuffers[channel].data() + offset, samples, sizeof(float) * length);
		#else
			for (uint i = 0; i < length; i++) {
				mBuffers[channel][i + offset] = samples[i];
			}
		#endif
	}

	/**
	 * Set a single channel from a double array
	 * @param samples An Array containing the interleaved audio samples
	 * @param channel Channelindex
	 * @param length The length (single channel)
	 */
	void set(const double* samples, const uchar channel, uint length, const uint offset = 0) {
		if (mChannels <= channel) { return; }
		length = std::min(length - offset, size() - offset);
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
	void set(T** const samples, uchar channels, uint length, const uint offset = 0) {
		for (uchar c = 0; c < channels; c++) {
			set(samples[c], c, length, offset);
		}
	};

	/**
	 * @param samples A 1D Array containing the interleaved audio samples (float or double)
	 * @param channels Channel count
	 * @param length The length (single channel)
	 */
	template <typename T>
	void setFromInterleaved(const T* samples, const uchar channels, uint length, const uint offset = 0) {
		length = std::min(length - offset, size() - offset);
		for (uchar c = 0; c < std::min(channels, mChannels); c++) {
			for(uint i = 0, j = c; i < length; i++, j+= channels) {
				mBuffers[c][i + offset] = samples[j];
			}
		}
	}

	/**
	 * Set from a buffer object
	 */
	void set(const AudioBuffer& buffer, const uint offset = 0) {
		const uint length = buffer.size();
		for (uchar c = 0; c < buffer.channels(); c++) {
			set(buffer.get(c), c, length, offset);
		}
	};

	void resize(const uint length,  uchar channels) {
		if (channels == mChannels && mBuffers[0].size() == length) {
			return;
		}
		TKLB_ASSERT(channels <= MAX_CHANNELS)
		channels = std::min(channels, uchar(MAX_CHANNELS));
		for(uchar c = 0; c < channels; c++) {
			mBuffers[c].resize(length);
		}
		for (uchar c = channels; c < MAX_CHANNELS; c++) {
			mBuffers[c].resize(0);
		}
		mChannels = channels;
	}

	void resize(const AudioBuffer& buffer) {
		resize(buffer.size(), buffer.channels());
	}

	void clear() {
		resize(0, 0);
	}

	void add(const AudioBuffer& buffer, uint offset = 0) {
		const uint size = std::min(buffer.size() - offset, this->size() - offset);
		const uchar channels = std::min(buffer.channels(), this->channels());

		#ifndef TKLB_NO_INTRINSICS
			const uint stride = xsimd::simd_type<sample>::size;
			const uint vectorize = size - size % stride;
			for (uchar c = 0; c < channels; c++) {
				sample* out = &mBuffers[c][offset];
				const sample* in = buffer.get(c);
				for(uint i = 0; i < vectorize; i += stride) {
					xsimd::simd_type<sample> a = xsimd::load_aligned(in);
					xsimd::simd_type<sample> b = xsimd::load_aligned(out);
					xsimd::store_aligned(out, (a + b));
					in += stride;
					out += stride;
				}
				for(uint i = vectorize; i < size; i++) {
					(*out++) += (*in++);
				}
			}
		#else
			for (uchar c = 0; c < channels; c++) {
				sample* out = &mBuffers[c][offset];
				const sample* in = buffer.get(c);
				for(uint i = 0; i < size; i++) {
					(*out++) += (*in++);
				}
			}
		#endif
	}

	uchar channels() const {
		return mChannels;
	}

	uint size() const {
		return mBuffers[0].size();
	}

	sample* get(const uchar channel) {
		return mBuffers[channel].data();
	};

	const sample* get(const uchar channel) const {
		return mBuffers[channel].data();
	};

};

}

#endif // TKLB_AUDIOBUFFER
