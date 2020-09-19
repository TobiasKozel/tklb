#ifndef TKLB_AUDIOBUFFER
#define TKLB_AUDIOBUFFER

#include <cstring>
#include <algorithm>

#ifndef TKLB_NO_SIMD
	#include "../../external/xsimd/include/xsimd/xsimd.hpp"
#endif

#include "../THeapBuffer.h"
#include "../../util/TAssert.h"

namespace tklb {

/**
 * @brief Class for handling the most basic audio needs
 * Does convenient type conversions
 */
template <typename T>
class AudioBufferTpl {

public:
	using uchar = unsigned char;
	using uint = unsigned int;
#ifdef TKLB_MAXCHANNELS
	static constexpr uchar MAX_CHANNELS = TKLB_MAXCHANNELS;
#else
	static constexpr uchar MAX_CHANNELS = 2;
#endif

	/**
	 * @brief Sample type exposed for convenience
	 */
	using sample = T;

	template <class T2>
	/**
	 * @brief Aligned vector type
	 */
	using Buffer = HeapBuffer<T2
	#ifndef TKLB_NO_SIMD
		, xsimd::aligned_allocator<T2, XSIMD_DEFAULT_ALIGNMENT>
	#endif
	>;

#ifndef TKLB_NO_SIMD
	static constexpr uint stride = xsimd::simd_type<T>::size;
#endif

private:
	Buffer<T> mBuffers[MAX_CHANNELS];
	T* mRawBuffers[MAX_CHANNELS];
	uchar mChannels = 0;
	uint mSize = 0;
	uint mValidSize = 0;

public:
	/**
	 * @brief Only relevant for resampling and oversampling.
	 */
	uint sampleRate = 0;

	/**
	 * @brief Empty buffer with no memory allocated yet
	 */
	AudioBufferTpl() { };

	/**
	 * @brief Empty buffer with no memory allocated yet but channel count set
	 */
	AudioBufferTpl(const uchar channels) {
		mChannels = channels;
	};

	/**
	 * @brief Buffer with memory allocated
	 */
	AudioBufferTpl(const uchar channels, const uint length) {
		resize(length, channels);
	};

	/**
	 * @brief * Set a single channel from a array
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
	 * @brief Set multiple channels from a 2D array
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
	 * @brief Set from another buffer object
	 * e.g. offset=10 and length=15 will copy 15 samples into the buffer starting at the 10th sample
	 * @param buffer Source buffer object
	 * @param offset Start offset in the target buffer
	 * @param length Operation will stop at that sample
	 */
	template <typename T2>
	void set(const AudioBufferTpl<T2>& buffer, const uint offset = 0, uint length = 0) {
		length = length == 0 ? buffer.size() : length;
		for (uchar c = 0; c < buffer.channels(); c++) {
			set(buffer.get(c), c, length, offset);
		}
	};

	/**
	 * @brief Set the entire buffer to a constant value
	 */
	void set(T value = 0) {
		for (uchar c = 0; c < channels(); c++) {
			fill_n(mBuffers[c].data(), size(), value);
		}
	}

	/**
	 * @brief Set multiple channels from an interleaved array
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
	 * @brief Resizes the buffer to the desired length and channel count.
	 * @brief The desired length in Samples. 0 will deallocate.
	 * @brief Desired channel count. 0 will deallocate.
	 */
	void resize(const uint length, uchar channels) {
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
		mSize = length;
		mValidSize = std::min(mValidSize, mSize);
	}

	/**
	 * @brief Resize to the provided length and keep the channelcount.
	 * If the channel count was 0 it will assume 1 channel instead.
	 * @param length The desired length in samples. 0 will deallocate.
	 */
	void resize(const uint length) {
		resize(length, std::max(uchar(1), mChannels));
	}

	/**
	 * @brief Resize to match the provided buffer
	 */
	template <typename T2>
	void resize(const AudioBufferTpl<T2>& buffer) {
		resize(buffer.size(), buffer.channels());
	}

	/**
	 * @brief Add the provided buffer
	 * @param buffer Buffer to add onto self
	 * @param offset Where to start in the own buffer
	 * @param size How many samples to add from the source buffer
	 */
	template <typename T2>
	void add(const AudioBufferTpl<T2>& buffer, uint offset = 0, uint size = 0) {
		if (size == 0) {
			size = std::min(buffer.size() - offset, this->size() - offset);
		}
		const uchar channels = std::min(buffer.channels(), this->channels());

		#ifndef TKLB_NO_SIMD
			if (std::is_same<T2, T>::value) {
				const uint vectorize = size - size % stride;
				for (uchar c = 0; c < channels; c++) {
					T* out = &mBuffers[c][offset];
					const T* in = reinterpret_cast<const T*>(buffer.get(c));
					for(uint i = 0; i < vectorize; i += stride) {
						xsimd::simd_type<T> a = xsimd::load_aligned(in + i);
						xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
						xsimd::store_aligned(out + i, (a + b));
					}
					for(uint i = vectorize; i < size; i++) {
						out[i] += in[i];
					}
				}
				return;
			}
		#endif

		for (uchar c = 0; c < channels; c++) {
			T* out = &mBuffers[c][offset];
			const T2* in = buffer.get(c);
			for(uint i = 0; i < size; i++) {
				out[i] += in[i];
			}
		}
	}

	/**
	 * @brief Multiply two buffers
	 * @param buffer Buffer to multiply onto self
	 * @param offset Where to start in the own buffer
	 * @param size How many samples to multiply from the source buffer
	 */
	template <typename T2>
	void multiply(const AudioBufferTpl<T2>& buffer, uint offset = 0, uint size = 0) {
		if (size == 0) {
			size = std::min(buffer.size() - offset, this->size() - offset);
		}
		const uchar channels = std::min(buffer.channels(), this->channels());

		#ifndef TKLB_NO_SIMD
			if (std::is_same<T2, T>::value) {
				const uint vectorize = size - size % stride;
				for (uchar c = 0; c < channels; c++) {
					T* out = &mBuffers[c][offset];
					const T* in = reinterpret_cast<const T*>(buffer.get(c));
					for(uint i = 0; i < vectorize; i += stride) {
						xsimd::simd_type<T> a = xsimd::load_aligned(in + i);
						xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
						xsimd::store_aligned(out + i, (a * b));
					}
					for(uint i = vectorize; i < size; i++) {
						out[i] *= in[i];
					}
				}
				return;
			}
		#endif

		for (uchar c = 0; c < channels; c++) {
			T* out = &mBuffers[c][offset];
			const T2* in = buffer.get(c);
			for(uint i = 0; i < size; i++) {
				out[i] *= in[i];
			}
		}
	}

	/**
	 * @brief Mutliplies the content of the buffer with a constant
	 * @param value
	 */
	void multiply(T value) {
		const uint size = this->size();
		const uchar channels = this->channels();

		#ifndef TKLB_NO_SIMD
			const uint vectorize = size - size % stride;
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][0];
				for(uint i = 0; i < vectorize; i += stride) {
					xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
					xsimd::store_aligned(out + i, (b * value));
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

	/**
	 * @brief Adds a constant to the contents of the buffer
	 * @param value
	 */
	void add(T value) {
		const uint size = this->size();
		const uchar channels = this->channels();

		#ifndef TKLB_NO_SIMD
			const uint vectorize = size - size % stride;
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][0];
				for(uint i = 0; i < vectorize; i += stride) {
					xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
					xsimd::store_aligned(out + i, (b + value));
				}
				for(uint i = vectorize; i < size; i++) {
					out[i] += value;
				}
			}
		#else
			for (uchar c = 0; c < channels; c++) {
				T* out = &mBuffers[c][0];
				for(uint i = 0; i < size; i++) {
					out[i] += value;
				}
			}
		#endif
	}

	/**
	 * @brief Inject forgeign memory to be used by the buffer.
	 * Potentially dangerous but useful when splitting
	 * up channels for processing
	 */
	void inject(T* mem, const uint size, const uchar channel = 0) {
		mBuffers[channel].inject(mem, size);
		mSize = size;
		mValidSize = size;
	}

	/**
	 * @brief Returns the amount of channels
	 */
	uchar channels() const {
		return mChannels;
	}

	/**
	 * @brief Returns the allocated length of the buffer
	 */
	uint size() const {
		return mSize;
	}

	/**
	 * @brief fReturns the length of actually valid audio in the buffer
	 */
	uint validSize() const {
		return mValidSize ? mValidSize : size();
	}

	/**
	 * @brief Set the amount of valid samples currently in the buffer
	 * This is mostly a convenience flag since the actual size of the buffer may be larger
	 */
	void setValidSize(const uint v) {
		TKLB_ASSERT(size() >= v);
		mValidSize = v;
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

	/**
	 * @brief Returns a array owned by the object containing pointers to all the channels.
	 * <b>Don't</b> call this constantly, try to cache the returned value for reuse
	 */
	T** getRaw() {
		for (uchar c = 0; c < mChannels; c++) {
			mRawBuffers[c] = get(c);
		}
		return mRawBuffers;
	}

	/**
	 * @brief Fill the provided array with the contents of this buffer
	 * @param target The arry to fill
	 * @param channel Which source channel to use
	 * @param length The length of the output
	 */
	template <typename T2>
	void put(T2* target, uchar channel, uint length) const {
		if (mChannels <= channel) { return; }
		length = std::min(length, size());
		const T2* source = get(channel);
		if (std::is_same<T2, T>::value) {
			memcpy(target, source, sizeof(T) * length);
		} else {
			for (uint i = 0; i < length; i++) {
				target[i] = static_cast<T2>(source[i]);
			}
		}
	}

	/**
	 * @brief Fill the provided 2D array with the contents of this buffer
	 * @param target The arry to fill
	 * @param channels How many channels there are in the target buffer
	 * @param length The length of the output
	 */
	template <typename T2>
	void put(T2** target, const uchar channels, const uint length) const {
		for (uchar c = 0; c < channels; c++) {
			put(target[c], c, length);
		}
	}
};


typedef AudioBufferTpl<float> AudioBufferFloat;
typedef AudioBufferTpl<double> AudioBufferDouble;

// Default type
#ifdef TKLB_SAMPLE_FLOAT
	typedef AudioBufferFloat AudioBuffer;
#else
	typedef AudioBufferDouble AudioBuffer;
#endif

}

#endif // TKLB_AUDIOBUFFER
