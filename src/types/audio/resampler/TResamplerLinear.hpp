/**
 * @file TResamplerLinear.hpp
 * @author Tobias Kozel
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _TKLB_RESAMPLER_LINEAR
#define _TKLB_RESAMPLER_LINEAR

#include "./TIResampler.hpp"
#include "../TAudioBuffer.hpp"


namespace tklb {
	/**
	 * @brief Linear resampler class
	 *        TODO do some benchmarking to ensure stack is actually
	 *        worth the aditional template argument
	 *
	 * @tparam T sample type
	 * @tparam Buffer AudioBuffer
	 * @tparam MAX_CHANNELS maximum number of channels
	 */
	template <typename T, class Buffer = AudioBufferTpl<T>, int MAX_CHANNELS = 32>
	class ResamplerLinearTpl : IResamplerTpl<T> {
		using Size = typename Buffer::Size;
		using Channel = typename Buffer::Channel;

		Size mRateIn, mRateOut;
		double mFactor = 1.0;
		T mOffset = 0;
		T mLastFrame[MAX_CHANNELS]; ///< should probbaly move this onto the heap
	public:
		/**
		 * @brief setup the resampler
		 * @param rateIn Input sample rate
		 * @param rateOut Desired output samplerate
		 * @param maxBlock Not used/needed for simple algorithms.
		 * @param quality Not used.
		 * @return True on success
		 */
		bool init(
			Size rateIn, Size rateOut,
			Size maxBlock = 512, Channel channels = 2,
			Size quality = 5
		) override {
			(void) maxBlock;
			(void) channels;
			(void) quality;
			TKLB_ASSERT(channels <= MAX_CHANNELS)

			for (auto& i : mLastFrame) { i = 0.0; }

			mRateIn = rateIn;
			mRateOut = rateOut;
			mFactor = double(mRateIn) / double(mRateOut);
			return true;
		}

		Size process(const Buffer& in, Buffer& out) override {
			TKLB_ASSERT(in.sampleRate == mRateIn);
			TKLB_ASSERT(out.sampleRate == mRateOut);
			TKLB_ASSERT(in.validSize() > 0)
			TKLB_ASSERT(estimateOut(in.validSize()) <= out.size())

			const Size countIn = in.validSize();
			Size countOut = 0;

			for (Channel c = 0; c < in.channels(); c++) {
				Size output = 0;								// index in output buffer
				T last = mLastFrame[c];							// last sample

				for (; output < out.size(); output++) {
					const T position = output * mFactor;		// index in input buffer, somewhere between two samples
					const T lastPosition = Size(position);		// next sample index in the input buffer
					T mix = position - lastPosition;			// mix factor between first and second sample

					const Size lastIndex = lastPosition;

					if (countIn < lastPosition) { break; }

					const T next = in[c][lastIndex];
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

		Size getLatency() const override {
			return 1; // lerp wil be one sample behind
		};

		Size estimateNeed(const Size out) const override {
			return tklb::round(out * mFactor);
		}

		Size estimateOut(const Size in) const override {
			return tklb::round(in * (double(mRateOut) / double(mRateIn)));
		}


		bool isInitialized() const override {
			return true;
		};

		Size calculateBufferSize(Size in) const override {
			return estimateOut(in) + 10;
		}
	};

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ResamplerLinear = ResamplerLinearTpl<float>;
	#else
		using ResamplerLinear = ResamplerLinearTpl<double>;
	#endif

} // namespace

#endif // _TKLB_RESAMPLER_LINEAR
