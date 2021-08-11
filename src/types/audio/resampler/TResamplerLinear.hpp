#ifndef TKLBZ_RESAMPLER_LINEAR
#define TKLBZ_RESAMPLER_LINEAR

#include "../TAudioBuffer.hpp"

namespace tklb {
	template <typename T>
	class ResamplerLinearTpl {
		using uchar = unsigned char;
		using uint = unsigned int;
		using Buffer = AudioBufferTpl<T>;
		using Size = typename Buffer::Size;

		uint mRateIn, mRateOut;
		T mOffset = 0;
		T mLastFrame[Buffer::MAX_CHANNELS];
		Buffer mBuffer;
	public:
		ResamplerLinearTpl(uint rateIn, uint rateOut, uint maxBlock = 512, uchar quality = 5) {
			for (auto& i : mLastFrane) { i = 0.0; }
			init(rateIn, rateOut, maxBlock, quality);
		}

		/**
		 * @brief setup the resampler
		 * @param rateIn Input sample rate
		 * @param rateOut Desired output samplerate
		 * @param maxBlock The maximum blocksize beeing passed into process().
		 * Only relevant when doing non float resampling to allocate space for the
		 * conversion buffers
		 * @param quality Quality factor from 1-10. Higher results in better quality and higher CPU usage. Depending on implementataion may not do anything.
		 * @return True on success
		 */
		bool init(uint rateIn, uint rateOut, uint maxBlock = 512, uchar quality = 5) {

		}

		/**
		 * @brief Resample function
		 * Make sure the out buffer has enough space
		 */
		Size process(const Buffer& in, Buffer& out) {
			// mBuffer.set(in); // not needed without lowpass
			const T factor = T(mRateIn) / T(mRateOut);
			const Size countIn = in.validSize();
			TKLB_ASSERT(countIn * factor < out.size()) // not enough size in out buffer
			Size countOut = 0;
			const T offset = mOffset;

			for (int c = 0; c < in.channels(); c++) {
				Size output = 0;									// index in output buffer
				T last = mLastFrame[c];								// last sample
				T mix = 0.0;
				for (; output < out.size(); output++) {
					const T position = output * factor + offset;	// index in input buffer, somewhere between two samples
					const T lastPosition = std::floor(position);	// next sample index in the input buffer
					const Size lastIndex = lastPosition;
					const Size nextIndex = lastPosition + 1;		// next sample index in the input buffer this is the one we need to fetch

					if (countIn <= nextIndex) { break; }

					mix = index - lastIndex;				// mix factor between first and second sample
					const T next = in[c][lastPosition];
					out[c][o] = last + mix * (next - last);
					// out[c][o] = next * mix + last * (T(1.0) - mix);
					last = next;
				}
				mLastFrame[c] = last;
				offset = mix;
				countOut = o;
			}
			out.setValidSize(countOut);
			return countOut;

			if (mRateIn < mRateOut) {
				// TODO tklb lerp
				// TODO tklb lowpass
			} else {
				// TODO tklb lowpass
				// TODO tklb lerp
			}
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
		Size estimateNeed(const Size in) {
			return in * (mRateIn / double(mRateOut));
		}

		bool isInitialized() const {
			return true;
		};

		/**
		 * @brief Calculate a buffersize fit for the resampled result.
		 * Also adds a bit of padding.
		 */
		static Size calculateBufferSize(uint rateIn, uint rateOut, Size initialSize) {
			return ceil(initialSize * (rateOut / double(rateIn))) + 10;
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

			buffer.resize(calculateBufferSize(rateIn, rateOut, samples));

			ResamplerLinearTpl<T> resampler;
			resampler.init(rateIn, rateOut, copy.size(), quality);
			resampler.process(copy, buffer);
		}
	};

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ResamplerLinear = ResamplerLinearTpl<float>;
	#else
		using ResamplerLinear = ResamplerLinearTpl<double>;
	#endif

} // namespace

#endif // TKLBZ_RESAMPLER_LINEAR
