#ifndef TKLB_AUDIOBUFFER
#define TKLB_AUDIOBUFFER

#include <vector>
#include <cstring>

#ifndef TKLB_NO_SIMD
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
	using T = float;
#else
	using T = double;
#endif

	template <class T2>
	/**
	 * Aligned vector type
	 */
	using Buffer = std::vector<T2
	#ifndef TKLB_NO_SIMD
		, xsimd::aligned_allocator<T2, XSIMD_DEFAULT_ALIGNMENT>
	#endif
	>;

private:
	Buffer<T> mBuffers[MAX_CHANNELS];
	T* mRawBuffers[MAX_CHANNELS];
	uchar mChannels = 0;

public:
	AudioBuffer() {
		std::fill_n(mRawBuffers, MAX_CHANNELS, nullptr);
	};

	/**
	 * Set a single channel from a double array
	 * @param samples An Array containing the interleaved audio samples
	 * @param channel Channelindex
	 * @param length The length (single channel)
	 */
	template <typename T2>
	void set(const T2* samples, const uchar channel, uint length, const uint offset = 0) {
		if (mChannels <= channel) { return; }
		length = std::min(length - offset, size() - offset);
		if (std::is_same<T2, T>::value) {
			memcpy(mBuffers[channel].data() + offset, samples, sizeof(T) * length);
		} else {
			for (uint i = 0; i < length; i++) {
				mBuffers[channel][i + offset] = static_cast<T>(samples[i]);
			}
		}
	}

	/**
	 * @param samples A 2D Array containing the audio samples (float or double)
	 * @param channels Channel count
	 * @param length The length (single channel)
	 */
	template <typename T2>
	void set(T2** const samples, uchar channels, uint length, const uint offset = 0) {
		for (uchar c = 0; c < channels; c++) {
			set(samples[c], c, length, offset);
		}
	};

	/**
	 * @param samples A 1D Array containing the interleaved audio samples (float or double)
	 * @param channels Channel count
	 * @param length The length (single channel)
	 */
	template <typename T2>
	void setFromInterleaved(const T2* samples, const uchar channels, uint length, const uint offset = 0) {
		length = std::min(length - offset, size() - offset);
		for (uchar c = 0; c < std::min(channels, mChannels); c++) {
			for(uint i = 0, j = c; i < length; i++, j+= channels) {
				mBuffers[c][i + offset] = static_cast<T>(samples[j]);
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

		#ifndef TKLB_NO_SIMD
			const uint stride = xsimd::simd_type<T>::size;
			const uint vectorize = size - size % stride;
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][offset];
				const T* in = buffer.get(c);
				for(uint i = 0; i < vectorize; i += stride) {
					xsimd::simd_type<T> a = xsimd::load_aligned(in);
					xsimd::simd_type<T> b = xsimd::load_aligned(out);
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
				T* out = &mBuffers[c][offset];
				const T* in = buffer.get(c);
				for(uint i = 0; i < size; i++) {
					(*out++) += (*in++);
				}
			}
		#endif
	}

	void multiply(const AudioBuffer& buffer, uint offset = 0) {
		const uint size = std::min(buffer.size() - offset, this->size() - offset);
		const uchar channels = std::min(buffer.channels(), this->channels());

		#ifndef TKLB_NO_SIMD
			const uint stride = xsimd::simd_type<T>::size;
			const uint vectorize = size - size % stride;
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][offset];
				const T* in = buffer.get(c);
				for(uint i = 0; i < vectorize; i += stride) {
					xsimd::simd_type<T> a = xsimd::load_aligned(in);
					xsimd::simd_type<T> b = xsimd::load_aligned(out);
					xsimd::store_aligned(out, (a * b));
					in += stride;
					out += stride;
				}
				for(uint i = vectorize; i < size; i++) {
					(*out++) *= (*in++);
				}
			}
		#else
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][offset];
				const T* in = buffer.get(c);
				for(uint i = 0; i < size; i++) {
					(*out++) *= (*in++);
				}
			}
		#endif
	}

	void multiply(T value) {
		const uint size = this->size();
		const uchar channels = this->channels();

		#ifndef TKLB_NO_SIMD
			const uint stride = xsimd::simd_type<T>::size;
			const uint vectorize = size - size % stride;
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][0];
				// xsimd::simd_type<T> a = xsimd::load_aligned(in);
				for(uint i = 0; i < vectorize; i += stride) {
					xsimd::simd_type<T> b = xsimd::load_aligned(out);
					xsimd::store_aligned(out, (b * value));
					out += stride;
				}
				for(uint i = vectorize; i < size; i++) {
					out[i] *= value;
				}
			}
		#else
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][0];
				for(uint i = 0; i < size; i++) {
					out[i] *= value;
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

	T* get(const uchar channel) {
		return mBuffers[channel].data();
	};

	const T* get(const uchar channel) const {
		return mBuffers[channel].data();
	};

	const T* operator[](const uchar channel) const {
		return get(channel);
	}

	T* operator[](const uchar channel) {
		return get(channel);
	}

	T** getRaw() {
		for (uchar c = 0; c < MAX_CHANNELS; c++) {
			mRawBuffers[c] = get(c);
		}
		return mRawBuffers;
	}
};

}

#endif // TKLB_AUDIOBUFFER
