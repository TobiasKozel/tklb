#ifndef _TKLB_AUDIOBUFFER
#define _TKLB_AUDIOBUFFER

#include "../../memory/TMemory.hpp"
#include "../../util/TMath.hpp"

#ifndef TKLB_NO_SIMD
	#include "../../../external/xsimd/include/xsimd/xsimd.hpp"
#endif

#include "../THeapBuffer.hpp"
#include "../../util/TTraits.hpp"
#include "../../util/TLimits.hpp"
#include "../../util/TAssert.h"


namespace tklb {
#ifdef TKLB_NO_SIMD
	constexpr SizeT DEFAULT_ALIGNMENT_BYTES = 16; // default to sse2 alignment
#else
	constexpr SizeT DEFAULT_ALIGNMENT_BYTES = xsimd::default_arch::alignment();
#endif // TKLB_NO_SIMD

	/**
	 * @brief Class for handling the most basic audio needs
	 * @details Does convenient type conversions
	 * TODO check if using a single buffer instead of one for each channel
	 * improves performance
	 *
	 * @tparam T Sample type. Can be anything traits::IsArithmetic
	 * @tparam STORAGE Storage type, tklb::HeapBuffer for now since there are a few things missing in a std::vector
	 */
	template <typename T, class STORAGE = HeapBuffer<T, DEFAULT_ALIGNMENT_BYTES>>
	class AudioBufferTpl {
		static_assert(traits::IsFloat<T>::value, "Need arithmetic type.");
	public:
		using Sample = T;
		using Storage = STORAGE;
		using Channel = unsigned char;

		// TODO higher sample rates wont work, maybe use enum
		using SampleRate = unsigned short;
		using Size = typename Storage::Size;

	#ifndef TKLB_NO_SIMD
		static constexpr Size Stride = xsimd::simd_type<T>::size;
	#endif

	private:
		Storage mBuffer;
		Size mValidSize = 0;
		Channel mChannels = 0;

	public:
		/**
		 * @brief Only relevant for resampling and oversampling.
		 * TODO move this out since it not always needed
		 */
		 SampleRate sampleRate = 0;

		/**
		 * @brief Empty buffer with no memory allocated yet
		 */
		AudioBufferTpl() = default;

		/**
		 * @brief Buffer with memory allocated
		 */
		AudioBufferTpl(const Size length, const Channel channels) {
			resize(length, channels);
		}

		~AudioBufferTpl() {
			TKLB_ASSERT_STATE(mChannels = 0;)
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
		 * @brief Figure out if conversions between types like float -> int or int -> short
		 * need scaling applied
		 * @tparam T2 Other type compared to own type
		 * @return true Scaling needs to be applied
		 * @return false No scaling needs to be applied
		 * @see getConversionScale
		 */
		template <typename T2>
		static constexpr bool needsScaling() {
			return
				(traits::IsFloat<T>::value && traits::IsFloat<T2>::value) ?
					false :
					limits::max<T>::value != limits::max<T2>::value;
		}

		/**
		 * @brief calculate the amplitude scale factor needed between the types
		 * @tparam T1 Source type
		 * @tparam T2 target type
		 * @tparam Ratio Floating point type since the value can be smaller than 1
		 * @return constexpr Ratio
		 */
		template <typename T1, typename T2, typename Ratio = float>
		static constexpr Ratio getConversionScale() {
			return
				(traits::IsFloat<T2>::value ?
					Ratio(1.0) : Ratio(limits::max<T2>::value) - Ratio(1))
				/ // --------------------------------------------------------
				(traits::IsFloat<T1>::value ?
					Ratio(1.0) : Ratio(limits::max<T1>::value) - Ratio(1));
		}

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
			const Channel channel = 0,
			const Size offsetDst = 0
		) {
			static_assert(traits::IsArithmetic<T2>::value, "Need arithmetic type.");
			if (channels() <= channel) { return; }
			TKLB_ASSERT(size() >= offsetDst)
			length = min(length, size() - offsetDst);
			T* out = get(channel);
			if (traits::IsSame<T2, T>::value) {
				// Types are identical
				memory::copy(out + offsetDst, samples, sizeof(T) * length);
			} else {
				if (!needsScaling<T2>()) {
					// Both floats means they need to be converted but not scaled
					for (Size i = 0; i < length; i++) {
						out[i + offsetDst] = static_cast<T>(samples[i]);
					}
				} else {
					// We also need to scale
					// float -> int, int -> float, short -> int
					const auto scale = getConversionScale<T2, T>();
					for (Size i = 0; i < length; i++) {
						out[i + offsetDst] = static_cast<T>(samples[i] * scale);
					}
				}
			}
		}

		/**
		 * @brief Set multiple channels from a 2D array
		 * @param samples A 2D Array containing the audio samples (float or double)
		 * @param length Samples to copy in (single channel)
		 * @param channelCount Channel count
		 * @param offsetSrc Offset in the source buffer
		 * @param offsetDst Offset in the destination buffer
		 */
		template <typename T2>
		void set(
			T2** const samples,
			const Size length,
			Channel chan = 0,
			const Size offsetSrc = 0,
			const Size offsetDst = 0
		) {
			static_assert(traits::IsArithmetic<T2>::value, "Need arithmetic type.");
			if (chan == 0) {
				chan = channels();
			}

			chan == min(chan, channels());

			for (Channel c = 0; c < chan; c++) {
				set(samples[c] + offsetSrc, length, c, offsetDst);
			}
		}

		/**
		 * @brief Set from another buffer object, will not adjust size and channel count!
		 * Use clone() instead.
		 * e.g. offset=10 and length=15 will copy 15 samples into the buffer starting at the 10th sample
		 * @param buffer Source buffer object
		 * @param length Samples to copy in
		 * @param offsetSrc Start offset in the source buffer
		 * @param offsetDst Start offset in the target buffer
		 */
		template <typename T2, class STORAGE2>
		void set(
			const AudioBufferTpl<T2, STORAGE2>& buffer,
			Size length = 0,
			const Size offsetSrc = 0,
			const Size offsetDst = 0
		) {
			length = length == 0 ? buffer.validSize() : length;
			for (Channel c = 0; c < buffer.channels(); c++) {
				set(buffer[c] + offsetSrc, length, c, offsetDst);
			}
		}

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
			length = min(size() - offsetDst, length ? length : size());
			for (Channel c = 0; c < channels(); c++) {
				memory::set<T>(get(c) + offsetDst, length, value);
			}
		}

		/**
		 * @brief Set multiple channels from an interleaved array
		 * @param samples A 1D Array containing the interleaved audio samples
		 * @param length The length of a single channel.
		 *               This means samples is (length * chan) long
		 * @param chan Channel count
		 * @param offsetSrc Start offset in the source buffer (from 0 to length)
		 * @param offsetDst Start offset in the target buffer
		 */
		template <typename T2>
		void setFromInterleaved(
			const T2* samples,
			Size length,
			const Channel chan,
			Size offsetSrc = 0,
			const Size offsetDst = 0
		) {
			static_assert(traits::IsArithmetic<T2>::value, "Need arithmetic type.");
			TKLB_ASSERT(size() >= offsetDst)
			length = min(size() - offsetDst, length);
			offsetSrc *= chan;
			for (Channel c = 0; c < min(chan, channels()); c++) {
				T* out = get(c);
				if (!needsScaling<T2>()) {
					for(Size i = 0, j = c + offsetSrc; i < length; i++, j+= chan) {
						out[i + offsetDst] = static_cast<T>(samples[j]);
					}
				} else {
					const auto scale = getConversionScale<T2, T>();
					for(Size i = 0, j = c + offsetSrc; i < length; i++, j+= chan) {
						out[i + offsetDst] = static_cast<T>(samples[j] * scale);
					}
				}
			}
			mValidSize = offsetDst + length;
		}


		/**
		 * @brief Match the size of the provided buffer and copy the contents
		 */
		template <typename T2, class STORAGE2>
		void clone(const AudioBufferTpl<T2, STORAGE2>& buffer) {
			resize(buffer);
			set(buffer);
		}

		/**
		 * @brief ! Will not keep the contents! Resizes the buffer to the
		 *        desired length and channel count.
		 * @details If the validSize = 0 it will be set to the new size
		 *          for convenience. The size reported back when calling
		 *          size is can be larger if the data is aligned.
		 * @param length The desired length in Samples. 0 will deallocate.
		 * @param chan Desired channel count. 0 will deallocate.
		 */
		bool resize(const Size length, Channel chan) {
			if (chan == channels() && size() == length) { return true; }
			// We need to ensure each channel is aligned so
			// we add some padding after each channel
			const auto elementAlign = mBuffer.closestChunkSize(
				length, mBuffer.Alignment / sizeof(T)
			);
			mBuffer.resize(0); // deallocate so we don't copy old misaligned signal over
			mBuffer.resize(chan * elementAlign);

			mChannels = chan;

			if (mValidSize == 0) {
				mValidSize = length;
			} else {
				mValidSize = min(mValidSize, length);
			}
			return true;
		}

		/**
		 * @brief Resize to the provided length and keep the channelcount.
		 *        If the channel count was 0 it will assume 1 channel instead.
		 * @param length The desired length in samples. 0 will deallocate.
		 */
		void resize(const Size length) {
			resize(length, max(Channel(1), channels()));
		}

		/**
		 * @brief Resize to match the provided buffer
		 */
		template <typename T2, class STORAGE2>
		void resize(const AudioBufferTpl<T2, STORAGE2>& buffer) {
			resize(buffer.size(), buffer.channels());
		}

		/**
		 * @brief Add the provided buffer
		 * @param buffer Source buffer object
		 * @param length Samples to add from the source buffer
		 * @param offsetSrc Start offset in the source buffer
		 * @param offsetDst Start offset in the target buffer
		 */
		template <typename T2, class STORAGE2>
		void add(
			const AudioBufferTpl<T2, STORAGE2>& buffer,
			Size length = 0,
			Size offsetSrc = 0,
			Size offsetDst = 0
		) {
			TKLB_ASSERT(validSize() >= offsetDst)
			TKLB_ASSERT(buffer.validSize() >= offsetSrc)
			length = length == 0 ? buffer.validSize() - offsetSrc : length;
			length = min(buffer.validSize() - offsetSrc, validSize() - offsetDst);
			const Channel channelCount = min(buffer.channels(), channels());

			#ifndef TKLB_NO_SIMD
				if (traits::IsSame<T2, T>::value) {
					const Size vectorize = length - (length % Stride);
					for (Channel c = 0; c < channelCount; c++) {
						T* out = get(c) + offsetDst;
						const T* in = reinterpret_cast<const T*>(buffer[c]) + offsetSrc;
						for(Size i = 0; i < vectorize; i += Stride) {
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
			for (Channel c = 0; c < channelCount; c++) {
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
		template <typename T2, class STORAGE2>
		void multiply(
			const AudioBufferTpl<T2, STORAGE2>& buffer,
			Size length = 0,
			Size offsetSrc = 0,
			Size offsetDst = 0
		) {
			TKLB_ASSERT(validSize() >= offsetDst)
			TKLB_ASSERT(buffer.validSize() >= offsetSrc)
			length = length == 0 ? buffer.validSize() - offsetSrc : length;
			length = min(buffer.validSize() - offsetSrc, validSize() - offsetDst);
			const Channel channelsCount = min(buffer.channels(), channels());

			#ifndef TKLB_NO_SIMD
				if (traits::IsSame<T2, T>::value) {
					const Size vectorize = length - (length % Stride);
					for (Channel c = 0; c < channelsCount; c++) {
						T* out = get(c) + offsetDst;
						const T* in = reinterpret_cast<const T*>(buffer[c]) + offsetSrc;
						for(Size i = 0; i < vectorize; i += Stride) {
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
			for (Channel c = 0; c < channelsCount; c++) {
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
			const Size length = validSize();

			#ifndef TKLB_NO_SIMD
				const Size vectorize = length - (length % Stride);
				for (Channel c = 0; c < channels(); c++) {
					T* out = get(c);
					for(Size i = 0; i < vectorize; i += Stride) {
						xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
						xsimd::store_aligned(out + i, (b * value));
					}
					for(Size i = vectorize; i < length; i++) {
						out[i] *= value;
					}
				}
			#else
				for (Channel c = 0; c < channels(); c++) {
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
			const Size length = validSize();

			#ifndef TKLB_NO_SIMD
				const Size vectorize = length - (length % Stride);
				for (Channel c = 0; c < channels(); c++) {
					T* out = get(c);
					for(Size i = 0; i < vectorize; i += Stride) {
						xsimd::simd_type<T> b = xsimd::load_aligned(out + i);
						xsimd::store_aligned(out + i, (b + value));
					}
					for(Size i = vectorize; i < length; i++) {
						out[i] += value;
					}
				}
			#else
				for (Channel c = 0; c < channels(); c++) {
					T* out = get(c);
					for(Size i = 0; i < length; i++) {
						out[i] += value;
					}
				}
			#endif
		}

		/**
		 * @brief Inject forgeign memory to be used by the buffer.
		 *        Potentially dangerous but useful when splitting up channels
		 *        for processing
		 * @param mem Const memory (No type conversions here)
		 *            All channels are stored sequentally (not interleaved)
		 *            and without padding in between.
		 * @param size Size of the entire buffer
		 * @param chan How many channels are contained in mem
		 */
		bool inject(T* mem, const Size size, const Channel chan) {
			if (!mBuffer.isAligned(mem)) {
				TKLB_ASSERT(false)
				return false;
			}
			TKLB_ASSERT(size % chan == 0)
			mBuffer.inject(mem, size);
			mValidSize = size / chan;
			mChannels = chan;
		}

		/**
		 * @brief Inject const forgeign memory to be used by the buffer.
		 *        Potentially dangerous but useful when splitting up channels
		 *        for processing
		 * @param mem Const memory (No type conversions here)
		 *            All channels are stored sequentally (not interleaved)
		 *            and without padding in between.
		 * @param size Size of the entire buffer
		 * @param chan How many channels are contained in mem
		 */
		bool inject(const T* mem, const Size size, const Channel chan) {
			if (!mBuffer.isAligned(mem)) {
				// injected memory needs to match the alignment
				TKLB_ASSERT(false)
				return false;
			}

			// We don't deal with padding between channels yet
			// see get() to find out why padding matters.
			TKLB_ASSERT(size % chan == 0)

			mBuffer.inject(mem, size);
			mValidSize = size / chan;
			mChannels = chan;
		}

		/**
		 * @brief Returns the amount of channels
		 */
		Channel channels() const { return mChannels; }

		/**
		 * @brief Returns the allocated length of the buffer
		 */
		inline Size size() const {
			return mBuffer.empty() ? 0 : mBuffer.size() / channels();
		}

		/**
		 * @brief Returns the length of actually valid audio in the buffer.
		 */
		Size validSize() const { return mValidSize; }

		/**
		 * @brief Set the amount of valid samples currently in the buffer
		 * This is mostly a convenience flag since the actual size of the buffer may be larger
		 */
		void setValidSize(const Size v) {
			TKLB_ASSERT(v <= size());
			mValidSize = min(size(), v);
		}

	#if defined(TKLB_MEMORY_CHECK) && false
		// TODO This isn't tested at all
		/**
		 * @brief Bound checking class
		 */
		struct AudioChannel {
			const T* start;
			const Size sampleCount;
			inline const T& operator[](const Size index) const {
				TKLB_ASSERT(index < sampleCount)
				return start[index];
			}
			inline T& operator[](const Size index) {
				TKLB_ASSERT(index < sampleCount)
				return start[index];
			}
		};

		inline AudioChannel get(const Channel channel) {
			TKLB_ASSERT(channel < channels())
			// We use the size of the buffer itself
			// since this will result in aligned addresses
			return mBuffer.data() + (channel * (mBuffer.size() / channels()));
		};

		inline const AudioChannel get(const Channel channel) const {
			TKLB_ASSERT(channel < channels())
			return mBuffer.data() + (channel * (mBuffer.size() / channels()));
		};

		inline const AudioChannel operator[](const Channel channel) const { return get(channel); }
		inline AudioChannel operator[](const Channel channel) { return get(channel); }

	#else // TKLB_MEMORY_CHECK
		inline T* get(const Channel channel) {
			TKLB_ASSERT(channel < channels())
			// We use the size of the buffer itself
			// since this will result in aligned addresses
			return mBuffer.data() + (channel * (mBuffer.size() / channels()));
		};

		inline const T* get(const Channel channel) const {
			TKLB_ASSERT(channel < channels())
			return mBuffer.data() + (channel * (mBuffer.size() / channels()));
		};

		inline const T* operator[](const Channel channel) const { return get(channel); }
		inline T* operator[](const Channel channel) { return get(channel); }
	#endif // TKLB_MEMORY_CHECK



		/**
		 * @brief Fills an 2d array of size maxChannels() with pointers to each channel
		 * @param put Pointers go here
		 */
		void getRaw(T** put) {
			for (Channel c = 0; c < channels(); c++) { put[c] = get(c); }
		}

		/**
		 * @brief Fills an 2d array of size maxChannels() with pointers to each channel
		 * @param put Pointers go here
		 */
		void getRaw(const T** put) const {
			for (Channel c = 0; c < channels(); c++) { put[c] = get(c); }
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
			const Channel channel = 0,
			const Size offset = 0
		) const {
			static_assert(traits::IsArithmetic<T2>::value, "Need arithmetic type.");
			if (channels() <= channel) { return 0; }
			const Size valid = validSize();
			TKLB_ASSERT(offset <= valid)
			length = length == 0 ? valid : length;
			length = min(length, valid - offset);

			const T* source = get(channel) + offset;
			if (traits::IsSame<T2, T>::value) {
				memory::copy(target, source, sizeof(T) * length);
			} else {
				if (!needsScaling<T2>()) {
					for (Size i = 0; i < length; i++) {
						target[i] = T2(source[i]);
					}
				} else {
					constexpr auto scale = getConversionScale<T, T2>();
					for (Size i = 0; i < length; i++) {
						target[i] = T2(source[i] * scale);
					}
				}
			}
			return length;
		}

		/**
		 * @brief Fill the provided 2D array with the contents of this buffer
		 * @param target The array to fill
		 * @param length The length of the output.
		 *               0 uses the validSize() of this buffer.
		 * @param chan How many channels there are in the target buffer.
		 *             0 uses channels() of this buffer.
		 * @return The amount of samples written, might be less than requested
		 */
		template <typename T2>
		Size put(
			T2** target,
			const Size length = 0,
			Channel chan = 0, // TODO make parameter order consistent
			const Size offset = 0
		) const {
			static_assert(traits::IsArithmetic<T2>::value, "Need arithmetic type.");
			Size res = 0;
			chan = (chan == 0) ? channels() : chan;
			for (Channel c = 0; c < chan; c++) {
				res = put<T2>(target[c], length, c, offset);
			}
			return res;
		}

		/**
		 * @brief Puts the contents interleaved in the target buffer
		 * @param buffer The array to fill with interleaved audio
		 * @param length Number of frames to interleave (not the length of the interleaved buffer)
		 *               0 uses the validSize() of this buffer.
		 * @param offset Offset for the sourcebuffer (this)
		 * TODO srcOffet and targetOffset maybe?
		 * @param chan maximum channels the output buffer can hold.
		 *             0 uses channels() of this buffer.
		 * @return Number of frames emitted
		 */
		template <typename T2>
		Size putInterleaved(
			T2* buffer,
			Size length = 0,
			const Size offset = 0,
			Channel chan = 0
		) const {
			static_assert(traits::IsArithmetic<T2>::value, "Need arithmetic type.");
			const Size valid = validSize();
			TKLB_ASSERT(offset <= valid)
			chan = (chan == 0) ? channels() : min(chan, channels());
			length = (length == 0) ? valid : length;
			length = min(valid - offset, length);
			Size out = 0;
			for (Channel c = 0; c < chan; c++) {
				Size j = c;
				const T* data = get(c);
				if (!needsScaling<T2>()) {
					for (Size i = 0; i < length; i++) {
						buffer[j] = T2(data[i + offset]);
						j += chan;
					}
				} else {
					constexpr auto scale = getConversionScale<T, T2>();
					for (Size i = 0; i < length; i++) {
						buffer[j] = T2(data[i + offset] * scale);
						j += chan;
					}
				}
				out += length;
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
