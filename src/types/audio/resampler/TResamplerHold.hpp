/**
 * @file TResamplerHold.hpp
 * @author Tobias Kozel
 * @brief
 * @version 0.1
 * @date 2023-02-20
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef _TKLB_RESAMPLER_HOLD
#define _TKLB_RESAMPLER_HOLD

#include "./TIResampler.hpp"
#include "../TAudioBuffer.hpp"

namespace tklb {
	/**
	 * @brief Simple sample and hold resampler.
	 *        This is very low quality and results in the
	 *        familiar down sampling effect often heard in bitcrusher effects.
	 * @tparam T Input/Output sample type
	 */
	template <typename T>
	class ResamplerHoldTpl : IResamplerTpl<T> {
		using Buffer = AudioBufferTpl<T>;
		using Channel = typename Buffer::Channel;
		using Size = typename Buffer::Size;
		Size mRateIn, mRateOut;
		double mFactor = 1.0;

	public:
		bool init(
			Size rateIn, Size rateOut,
			Size maxBlock = 512,
			Channel channels = 2,
			Size quality = 5
		) override {
			(void) maxBlock;
			(void) quality;
			(void) channels;
			mRateIn = rateIn;
			mRateOut = rateOut;
			mFactor = double(mRateIn) / double(mRateOut);
			return true;
		}

		Size process(const Buffer& in, Buffer& out) override {
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

		Size getLatency() const override { return 0; };

		Size estimateNeed(const Size out) const override {
			return tklb::round(out * mFactor);
		}

		Size estimateOut(const Size in) const override {
			return tklb::round(in * (double(mRateOut) / double(mRateIn)));
		}

		bool isInitialized() const override { return true; };

		Size calculateBufferSize(Size initialSize) const override {
			return estimateOut(initialSize) + 10;
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
