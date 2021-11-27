#ifndef _TKLB_AUDIOBUFFER
#define _TKLB_AUDIOBUFFER

#include <algorithm>
#include <type_traits>	// std::is_arithmetic

#include "../../memory/TMemory.hpp"
#include "../../util/TMath.hpp"

#ifndef TKLB_NO_SIMD
	#include "../../../external/xsimd/include/xsimd/xsimd.hpp"
#endif

#include "../THeapBuffer.hpp"
#include "../../util/TAssert.h"

#ifdef TKLB_MAXCHANNELS
	#if TKLB_MAXCHANNELS == 1
		#error "Setting TKLB_MAXCHANNELS lower than 2 will break FFTs and Convolution."
	#endif

	#if TKLB_MAXCHANNELS == 0
		#error "Setting TKLB_MAXCHANNELS to 0 is not an option."
	#endif
#endif

namespace tklb {

	/**
	 * @brief Class for handling the most basic audio needs
	 * Does convenient type conversions
	 * TODO check if using a single buffer instead of one for each channel
	 * improves performance
	 */
	template <typename T>
	class AudioBufferTpl {
		static_assert(std::is_arithmetic<T>::value, "Need arithmetic type.");
	public:
		/**
		 * @brief Sample type exposed for convenience
		 */
		using sample = T;

		template <class T2>
		/**
		 * @brief Aligned vector type
		 */
		using Buffer = HeapBuffer<T2, true>;

		using uchar = unsigned char;
		using uint = unsigned int;
		using ushort = unsigned short;
		using Size = typename Buffer<T>::Size;

	#ifdef TKLB_MAXCHANNELS
		static constexpr uchar MAX_CHANNELS = TKLB_MAXCHANNELS;
	#else
		static constexpr uchar MAX_CHANNELS = 2;
	#endif

	#ifndef TKLB_NO_SIMD
		static constexpr Size stride = xsimd::simd_type<T>::size;
	#endif

	private:
		Buffer<T> mBuffers[MAX_CHANNELS]; // TODO performance use a single linear buffer maybe
		Size mSize = 0; // TODO preformance throw this out and use heapbuffer size
		Size mValidSize = 0; // TODO use for set add and multiply
		uchar mChannels = 0;

	public:
		/**
		 * @brief Only relevant for resampling and oversampling.
		 * TODO higher sample rates wont work, maybe use enum
		 */
		ushort sampleRate = 0;

		/**
		 * @brief Empty buffer with no memory allocated yet
		 */
		AudioBufferTpl() { }

		/**
		 * @brief Buffer with memory allocated
		 */
		AudioBufferTpl(const Size length, const uchar channels) {
			resize(length, channels);
		}

		~AudioBufferTpl() {
			TKLB_ASSERT_STATE(mChannels = 0;)
			TKLB_ASSERT_STATE(mSize = 0;)
			TKLB_ASSERT_STATE(mValidSize = 0;)
		}

		/**
		 * @brief move contructor is implicitly deleted
		 * Don't know why
		 */
		AudioBufferTpl(AudioBufferTpl&& source) = default;

		AudioBufferTpl& operator= (AudioBufferTpl&& source) = default;

		AudioBufferTpl& operator= (const AudioBufferTpl&) = delete;
		AudioBufferTpl(const AudioBufferTpl&) = delete;
		AudioBufferTpl(const AudioBufferTpl*) = delete;


		/**
		 * @brief Set a single channel from an array
		 * @param samples An Array containing the audio samples
		 * @param length Amount of sample to copy
		 * @param channel Channel index
		 * @param offsetDst The offset for the destination buffer (this)
		 */
		template <typename T2>
		void set(
			const T2* samples,
			Size length,
			const uchar channel = 0,
			const Size offsetDst = 0
		) {
			static_assert(std::is_arithmetic<T2>::value, "Need arithmetic type.");
			if (mChannels <= channel) { return; }
			TKLB_ASSERT(size() >= offsetDst)
			length = std::min(length, size() - offsetDst);
			T* out = get(channel);
			if (std::is_same<T2, T>::value) {
				memory::copy(out + offsetDst, samples, sizeof(T) * length);
			} else {
				for (Size i = 0; i < length; i++) {
					out[i + offsetDst] = static_cast<T>(samples[i]);
				}
			}
		}

		/**
		 * @brief Set multiple channels from a 2D array
		 * @param samples A 2D Array containing the audio samples (float or double)
		 * @param length Samples to copy in (single channel)
		 * @param channels Channel count
		 * @param offsetSrc Offset in the source buffer
		 * @param offsetDst Offset in the destination buffer
		 */
		template <typename T2>
		void set(
			T2** const samples,
			const Size length,
			const uchar channels,
			const Size offsetSrc = 0,
			const Size offsetDst = 0
		) {
			static_assert(std::is_arithmetic<T2>::value, "Need arithmetic type.");
			for (uchar c = 0; c < channels; c++) {
				set(samples[c] + offsetSrc, length, c, offsetDst);
			}
		};

		/**
		 * @brief Set from another buffer object, will not adjust size and channel count!
		 * Use clone() instead.
		 * e.g. offset=10 and length=15 will copy 15 samples into the buffer starting at the 10th sample
		 * @param buffer Source buffer object
		 * @param length Samples to copy in
		 * @param offsetSrc Start offset in the source buffer
		 * @param offsetDst Start offset in the target buffer
		 */
		template <typename T2>
		void set(
			const AudioBufferTpl<T2>& buffer,
			Size length = 0,
			const Size offsetSrc = 0,
			const Size offsetDst = 0
		) {
			length = length == 0 ? buffer.size() : length;
			for (uchar c = 0; c < buffer.channels(); c++) {
				set(buffer[c] + offsetSrc, length, c, offsetDst);
			}
		};

		/**
		 * @brief Set the entire buffer to a constant value
		 * @param value Value to fill the buffer with
		 * @param length Samples to set. 0 Sets all
		 * @param offsetDst Start offset in the target buffer
		 */
		void set(
			T value = 0,
			Size length = 0,
			const Size offsetDst = 0
		) {
			TKLB_ASSERT(size() >= offsetDst)
			length = std::min(size() - offsetDst, length ? length : size());
			for (uchar c = 0; c < channels(); c++) {
				std::fill_n(get(c) + offsetDst, length, value);
			}
		}

		/**
		 * @brief Set multiple channels from an interleaved array
		 * @param samples A 1D Array containing the interleaved audio samples (float or double)
		 * @param length The length of a single channel
		 * @param channels Channel count
		 * @param offsetSrc Start offset in the source buffer
		 * @param offsetDst Start offset in the target buffer
		 */
		template <typename T2>
		void setFromInterleaved(
			const T2* samples,
			Size length,
			const uchar channels,
			Size offsetSrc = 0,
			const Size offsetDst = 0
		) {
			static_assert(std::is_arithmetic<T2>::value, "Need arithmetic type.");
			TKLB_ASSERT(size() >= offsetDst)
			length = std::min(size() - offsetDst, length);
			offsetSrc *= channels;
			for (uchar c = 0; c < std::min(channels, mChannels); c++) {
				T* out = get(c);
				for(Size i = 0, j = c + offsetSrc; i < length; i++, j+= channels) {
					out[i + offsetDst] = static_cast<T>(samples[j]);
				}
			}
			mValidSize = length;
		}


		/**
		 * @brief Match the size of the provided buffer and copy the contents
		 */
		template <typename T2>
		void clone(const AudioBufferTpl<T2>& buffer) {
			resize(buffer);
			set(buffer);
		}

		/**
		 * @brief Resizes the buffer to the desired length and channel count.
		 * If the validSize = 0 it will be set to the new size for convenience.
		 * @param length The desired length in Samples. 0 will deallocate.
		 * @param channels Desired channel count. 0 will deallocate.
		 */
		void resize(const Size length, uchar channels) {
			if (channels == mChannels && size() == length) {
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
			if (mValidSize == 0) {
				mValidSize = mSize;
			} else {
				mValidSize = std::min(mValidSize, mSize);
			}
		}

		/**
		 * @brief Resize to the provided length and keep the channelcount.
		 * If the channel count was 0 it will assume 1 channel instead.
		 * @param length The desired length in samples. 0 will deallocate.
		 */
		void resize(const Size length) {
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
		 * @param buffer Source buffer object
		 * @param length Samples to add from the source buffer
		 * @param offsetSrc Start offset in the source buffer
		 * @param offsetDst Start offset in the target buffer
		 */
		template <typename T2>
		void add(
			const AudioBufferTpl<T2>& buffer,
			Size length = 0,
			Size offsetSrc = 0,
			Size offsetDst = 0
		) {
			TKLB_ASSERT(size() >= offsetDst)
			TKLB_ASSERT(buffer.size() >= offsetSrc)
			length = length == 0 ? buffer.size() - offsetSrc : length;
			length = std::min(buffer.size() - offsetSrc, size() - offsetDst);
			const uchar channelCount = std::min(buffer.channels(), channels());

			#ifndef TKLB_NO_SIMD
				if (std::is_same<T2, T>::value) {
					const Size vectorize = length - (length % stride);
					for (uchar c = 0; c < channelCount; c++) {
						T* out = get(c) + offsetDst;
						const T* in = reinterpret_cast<const T*>(buffer[c]) + offsetSrc;
						for(Size i = 0; i < vectorize; i += stride) {
							xsimd::simd_type<T> a = xsimd::load_aligned(in + i);
							xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
							xsimd::store_aligned(out + i, (a + b));
						}
						for(Size i = vectorize; i < length; i++) {
							out[i] += in[i];
						}
					}
					return;
				}
			#endif

			// If the type doen't match or simd is diabled
			for (uchar c = 0; c < channelCount; c++) {
				T* out = get(c) + offsetDst;
				const T2* in = buffer[c] + offsetSrc;
				for(Size i = 0; i < length; i++) {
					out[i] += in[i];
				}
			}
		}

		/**
		 * @brief Multiply two buffers
		 * @param buffer Source buffer object
		 * @param length Samples to multiply from the source buffer
		 * @param offsetIn Start offset in the source buffer
		 * @param offset Start offset in the target buffer
		 */
		template <typename T2>
		void multiply(
			const AudioBufferTpl<T2>& buffer,
			Size length = 0,
			Size offsetSrc = 0,
			Size offsetDst = 0
		) {
			TKLB_ASSERT(size() >= offsetDst)
			TKLB_ASSERT(buffer.size() >= offsetSrc)
			length = length == 0 ? buffer.size() - offsetSrc : length;
			length = std::min(buffer.size() - offsetSrc, size() - offsetDst);
			const uchar channelsCount = std::min(buffer.channels(), channels());

			#ifndef TKLB_NO_SIMD
				if (std::is_same<T2, T>::value) {
					const Size vectorize = length - (length % stride);
					for (uchar c = 0; c < channelsCount; c++) {
						T* out = get(c) + offsetDst;
						const T* in = reinterpret_cast<const T*>(buffer[c]) + offsetSrc;
						for(Size i = 0; i < vectorize; i += stride) {
							xsimd::simd_type<T> a = xsimd::load_aligned(in + i);
							xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
							xsimd::store_aligned(out + i, (a * b));
						}
						for(Size i = vectorize; i < length; i++) {
							out[i] *= in[i];
						}
					}
					return;
				}
			#endif

			// If the type doen't match or simd is diabled
			for (uchar c = 0; c < channelsCount; c++) {
				T* out = get(c) + offsetDst;
				const T2* in = buffer[c] + offsetSrc;
				for(Size i = 0; i < length; i++) {
					out[i] *= in[i];
				}
			}
		}

		/**
		 * @brief Mutliplies the content of the buffer with a constant
		 * @param value Constant to multiply the buffer with
		 */
		void multiply(T value) {
			const Size length = size();

			#ifndef TKLB_NO_SIMD
				const Size vectorize = length - (length % stride);
				for (uchar c = 0; c < channels(); c++) {
					T* out = get(c);
					for(Size i = 0; i < vectorize; i += stride) {
						xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
						xsimd::store_aligned(out + i, (b * value));
					}
					for(Size i = vectorize; i < length; i++) {
						out[i] *= value;
					}
				}
			#else
				for (uchar c = 0; c < channels(); c++) {
					T* out = get(c);
					for(Size i = 0; i < length; i++) {
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
			const Size length = size();

			#ifndef TKLB_NO_SIMD
				const Size vectorize = length - (length % stride);
				for (uchar c = 0; c < channels(); c++) {
					T* out = get(c);
					for(Size i = 0; i < vectorize; i += stride) {
						xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
						xsimd::store_aligned(out + i, (b + value));
					}
					for(Size i = vectorize; i < length; i++) {
						out[i] += value;
					}
				}
			#else
				for (uchar c = 0; c < channels(); c++) {
					T* out = get(c);
					for(Size i = 0; i < length; i++) {
						out[i] += value;
					}
				}
			#endif
		}

		/**
		 * @brief Inject forgeign memory to be used by the buffer.
		 * Potentially dangerous but useful when splitting up channels for processing
		 * @param mem Modifiable memory (No type conversions here)
		 * @param size Size of the buffer
		 * @param channel The channel index
		 */
		void inject(T* mem, const Size size, const uchar channel = 0) {
			TKLB_ASSERT(channel < MAX_CHANNELS)
			mBuffers[channel].inject(mem, size);
			mSize = size;
			mValidSize = size;
			if (mChannels < channel + 1) {
				mChannels = channel + 1;
			}
		}

		/**
		 * @brief Inject const forgeign memory to be used by the buffer.
		 * Potentially dangerous but useful when splitting up channels for processing
		 * @param mem Const memory (No type conversions here)
		 * @param size Size of the buffer
		 * @param channel The channel index
		 */
		void inject(const T* mem, const Size size, const uchar channel = 0) {
			TKLB_ASSERT(channel < MAX_CHANNELS)
			mBuffers[channel].inject(mem, size);
			mSize = size;
			mValidSize = size;
			if (mChannels < channel + 1) {
				mChannels = channel + 1;
			}
		}

		/**
		 * @brief Returns the amount of channels
		 */
		uchar channels() const { return mChannels; }

		/**
		 * @brief Returns the allocated length of the buffer
		 */
		Size size() const { return mSize; }

		/**
		 * @brief fReturns the length of actually valid audio in the buffer.
		 * TODO tklb make sure this is used consistently
		 */
		Size validSize() const { return mValidSize; }

		/**
		 * @brief Set the amount of valid samples currently in the buffer
		 * This is mostly a convenience flag since the actual size of the buffer may be larger
		 */
		void setValidSize(const Size v) {
			TKLB_ASSERT(v <= size());
			mValidSize = v;
		}

		T* get(const uchar channel) {
			TKLB_ASSERT(channel < mChannels)
			return mBuffers[channel].data();
		};

		const T* get(const uchar channel) const {
			TKLB_ASSERT(channel < mChannels)
			return mBuffers[channel].data();
		};

		const T* operator[](const uchar channel) const { return get(channel); }

		T* operator[](const uchar channel) { return get(channel); }

	private:
		void assertOnConstMem() {
			for (uchar c = 0; c < MAX_CHANNELS; c++) {
				// non const accessor will cause an assertsion
				T* mem = mBuffers[c].data();
			}
		}

	public:
		/**
		 * @brief Returns a array owned by the object containing pointers to all the channels.
		 */
		T** getRaw() {
			TKLB_ASSERT_STATE(assertOnConstMem())
			TKLB_ASSERT(false) // TODO
			return nullptr;
		}

		const T** getRaw() const {
			TKLB_ASSERT(false) // TODO
			return nullptr;
		}

		/**
		 * @brief Fill the provided array with the contents of this buffer
		 * @param target The arry to fill
		 * @param channel Which source channel to use
		 * @param length The length of the output
		 * @param offset Start offset in the source (this) buffer
		 * @return The amount of samples written, might be less than requested
		 */
		template <typename T2>
		Size put(
			T2* target,
			Size length = 0,
			const uchar channel = 0,
			const Size offset = 0
		) const {
			static_assert(std::is_arithmetic<T2>::value, "Need arithmetic type.");
			if (mChannels <= channel) { return 0; }
			const Size valid = validSize();
			TKLB_ASSERT(offset <= valid)
			length = length == 0 ? valid : length;
			length = std::min(length, valid - offset);
			const T2* source = get(channel) + offset;
			if (std::is_same<T2, T>::value) {
				memory::copy(target, source, sizeof(T) * length);
			} else {
				for (Size i = 0; i < length; i++) {
					target[i] = T2(source[i]);
				}
			}
			return length;
		}

		/**
		 * @brief Fill the provided 2D array with the contents of this buffer
		 * @param target The array to fill
		 * @param channels How many channels there are in the target buffer
		 * @param length The length of the output
		 * @return The amount of samples written, might be less than requested
		 */
		template <typename T2>
		Size put(T2** target,
			const Size length = 0,
			uchar channels = 0,
			const Size offset = 0
		) const {
			static_assert(std::is_arithmetic<T2>::value, "Need arithmetic type.");
			Size res = 0;
			channels = (channels == 0) ? mChannels : channels;
			for (uchar c = 0; c < channels; c++) {
				res = put(target[c], length, c, offset);
			}
			return res;
		}

		/**
		 * @brief Puts the interleaved contents in the target buffer
		 * @param buffer The array to fill with interleaved audio
		 * @param length Number of frames to interleave (not the length of the interleaved buffer)
		 * @param offset Offset for the sourcebuffer (this)
		 * @return Number of frames emitted
		 */
		template <typename T2>
		Size putInterleaved(
			T2* buffer,
			Size length = 0,
			const Size offset = 0
		) const {
			static_assert(std::is_arithmetic<T2>::value, "Need arithmetic type.");
			const Size valid = validSize();
			TKLB_ASSERT(offset <= valid)
			const uchar chan = channels();
			length = (length == 0) ? valid : length;
			length = std::min(valid - offset, length);
			Size out = 0;
			// TODO see how the cpu cache is handling this
			for (Size i = 0; i < length; i++) {
				for (uchar c = 0; c < chan; c++) {
					buffer[out] = T2(mBuffers[c][i + offset]);
					out++;
				}
			}
			return out / Size(channels());
		}
	};


	using AudioBufferFloat = AudioBufferTpl<float>;
	using AudioBufferDouble = AudioBufferTpl<double>;

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using AudioBuffer = AudioBufferTpl<float>;
	#else
		using AudioBuffer = AudioBufferTpl<double>;
	#endif

} // namespace tklb

#endif // _TKLB_AUDIOBUFFER
