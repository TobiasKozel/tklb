#ifndef _TKLB_RESAMPLER_HOLD
#define _TKLB_RESAMPLER_HOLD

#include <cmath>
#include "../TAudioBuffer.hpp"

namespace tklb {
	template <typename T>
	class ResamplerHoldTpl {
		using uchar = unsigned char;
		using uint = unsigned int;
		using Buffer = AudioBufferTpl<T>;
		using Size = typename Buffer::Size;
		uint mRateIn, mRateOut;
		double mFactor = 1.0;

	public:
		ResamplerHoldTpl(uint rateIn, uint rateOut, uint maxBlock = 512, uchar quality = 5) {
			init(rateIn, rateOut, maxBlock, quality);
		}

		/**
		 * @brief setup the resampler
		 * @param rateIn Input sample rate
		 * @param rateOut Desired output samplerate
		 * @param maxBlock Not used/needed for simple algorithms.
		 * @param quality Not used
		 * @return True on success
		 */
		bool init(uint rateIn, uint rateOut, uint maxBlock = 512, uchar quality = 5) {
			(void) maxBlock;
			(void) quality;
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
			const Size countIn = in.validSize();
			Size countOut = 0;

			for (int c = 0; c < in.channels(); c++) {
				Size output = 0;										// index in output buffer
				for (; output < out.size(); output++) {
					const Size index = tklb::round(output * mFactor);	// closest sample
					if (countIn <= index) { break; }
					out[c][output] = in[c][index];
				}
				countOut = output;
			}
			out.setValidSize(countOut);
			return countOut;
		}

		/**
		 * @brief Get the latency in samples
		 */
		int getLatency() const { return 0; };

		/**
		 * @brief Estimate how many samples need to be put in to get n samples out.
		 */
		Size estimateNeed(const Size out) const {
			return tklb::round(out * mFactor);
		}

		/**
		 * @brief Estimate how many sample will be emitted in the next step
		 */
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
		Size calculateBufferSize(Size initialSize) {
			return estimateOut(initialSize) + 10;
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

			ResamplerHoldTpl<T> resampler;
			resampler.init(rateIn, rateOut, copy.size(), quality);
			buffer.resize(resampler.calculateBufferSize(samples));

			resampler.process(copy, buffer);
		}
	};

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ResamplerHold = ResamplerHoldTpl<float>;
	#else
		using ResamplerHold = ResamplerHoldTpl<double>;
	#endif

} // namespace

#endif // _TKLB_RESAMPLER_HOLD
