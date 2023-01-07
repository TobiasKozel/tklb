#ifndef _TKLB_RESAMPLER_COSINE
#define _TKLB_RESAMPLER_COSINE

#include "../TAudioBuffer.hpp"

namespace tklb {
	template <typename T>
	class ResamplerCosineTpl {
		using uchar = unsigned char;
		using uint = unsigned int;
		using Buffer = AudioBufferTpl<T>;
		using Size = typename Buffer::Size;

		uint mRateIn, mRateOut;
		double mFactor = 1.0;
		T mOffset = 0;
		static constexpr Size MAX_CHANNELS = 32;
		T mLastFrame[MAX_CHANNELS];
	public:
		ResamplerCosineTpl(uint rateIn, uint rateOut, uint maxBlock = 512, uchar channels = 2, uchar quality = 5) {
			for (auto& i : mLastFrame) { i = 0.0; }
			init(rateIn, rateOut, maxBlock, channels, quality);
		}

		ResamplerCosineTpl() = default;

		/**
		 * @brief setup the resampler
		 * @param rateIn Input sample rate
		 * @param rateOut Desired output samplerate
		 * @param maxBlock Not used/needed for simple algorithms.
		 * @param quality Not used
		 * @return True on success
		 */
		bool init(uint rateIn, uint rateOut, uint maxBlock = 512, uchar channels = 2, uchar quality = 5) {
			(void) channels;
			(void) quality;
			(void) maxBlock;
			TKLB_ASSERT(channels <= MAX_CHANNELS)
			mRateIn = rateIn;
			mRateOut = rateOut;
			mFactor = double(mRateIn) / double(mRateOut);
			return true;
		}

		/**
		 * @brief Resample function
		 * Make sure the out buffer has enough space
		 */
		Size process(const Buffer& in, Buffer& out) {
			TKLB_ASSERT(in.sampleRate == mRateIn);
			TKLB_ASSERT(out.sampleRate == mRateOut);
			TKLB_ASSERT(in.validSize() > 0)
			TKLB_ASSERT(estimateOut(in.validSize()) <= out.size())

			const Size countIn = in.validSize();
			Size countOut = 0;
			// const T offset = mOffset;
			// T lastMix = 0;

			for (int c = 0; c < in.channels(); c++) {
				Size output = 0;									// index in output buffer
				T last = mLastFrame[c];								// last sample
				// T mix = 0.0;
				for (; output < out.size(); output++) {
					const T position = output * mFactor;			// index in input buffer, somewhere between two samples
					const T lastPosition = tklb::floor(position);	// next sample index in the input buffer
					const Size lastIndex = lastPosition;
					const Size nextIndex = lastPosition + 1;		// next sample index in the input buffer this is the one we need to fetch

					if (countIn <= nextIndex) { break; }

					const T next = in[c][lastIndex];
					T mix = 0.5 * (1.0 - tklb::cos((position - lastPosition) * 3.1416));
					out[c][output] = last + mix * (next - last);
					last = next;
				}
				mLastFrame[c] = last;
				// lastMix = mix;
				countOut = output;
			}
			// mOffset = lastMix;
			out.setValidSize(countOut);
			return countOut;
		}

		/**
		 * @brief Get the latency in samples
		 */
		int getLatency() const {
			return 1; // lerp wil be one sample behind
		};

		/**
		 * @brief Estimate how many samples need to be put in to get n samples out.
		 */
		Size estimateNeed(const Size out) const {
			return tklb::round(out * mFactor);
		}

		Size estimateOut(const Size in) const {
			return tklb::round(in * (double(mRateOut) / double(mRateIn)));
		}


		bool isInitialized() const {
			return true;
		};

		/**
		 * @brief Calculate a buffersize fit for the resampled result.
		 * Also adds a bit of padding.
		 */
		Size calculateBufferSize(Size in) {
			return estimateOut(in) + 10;
		}

		/**
		 * @brief Resamples the provided buffer from its sampleRate
		 * to the target rate
		 * @param buffer Audiobuffer to resample, set the rate of the buffer object
		 * @param rateOut Desired output samplerate in Hz
		 * @param quality Quality from 1-10
		 */
		static void resample(Buffer& buffer, const uint rateOut, const uchar quality = 5) {
			// TODO tklb compensate delay
			const uint rateIn = buffer.sampleRate;
			const Size samples = buffer.size();
			TKLB_ASSERT(rateIn > 0)
			// Make a copy, this could be skipped when a conversion to float is needed anyways
			Buffer copy;
			copy.resize(buffer);
			copy.set(buffer);
			copy.sampleRate = rateIn;
			copy.setValidSize(samples);

			ResamplerCosineTpl<T> resampler;
			resampler.init(rateIn, rateOut, copy.size(), quality);
			buffer.resize(resampler.calculateBufferSize(samples));

			resampler.process(copy, buffer);
		}
	};

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ResamplerCosine = ResamplerCosineTpl<float>;
	#else
		using ResamplerCosine = ResamplerCosineTpl<double>;
	#endif

} // namespace

#endif // _TKLB_RESAMPLER_COSINE
