#ifndef _TKLB_IRESAMPLER
#define _TKLB_IRESAMPLER

#include "../TAudioBuffer.hpp"

namespace tklb {
	/**
	 * @brief Common interface defined for all resamplers
	 * 	      Use the actual derived resampler type whenever
	 *        possible to allow the compiler to optimize better
	 *
	 * @tparam T
	 * @tparam Buffer
	 */
	template <typename T, class Buffer = AudioBufferTpl<T>>
	struct IResamplerTpl {
		using Size = typename Buffer::Size;
		using Channel = typename Buffer::Channel;

		/**
		 * @brief setup the resampler
		 * @param rateIn Input sample rate
		 * @param rateOut Desired output samplerate
		 * @param maxBlock Not used/needed for simple algorithms.
		 * @param quality Not used.
		 * @return True on success
		 */
		virtual bool init(
			Size rateIn, Size rateOut,
			Size maxBlock = 512,
			Channel channels = 2, Size quality = 5
		) = 0;

		/**
		 * @brief Resample function
		 *        Make sure the out buffer has enough space
		 */
		virtual Size process(const Buffer& in, Buffer& out) = 0;

		/**
		 * @brief Get the latency in samples
		 */
		virtual Size getLatency() const = 0;

		/**
		 * @brief Estimate how many samples need to be put in to get n samples out.
		 */
		virtual Size estimateNeed(const Size out) const = 0;

		/**
		 * @brief TODO
		 *
		 * @param in
		 * @return Size
		 */
		virtual Size estimateOut(const Size in) const = 0;


		/**
		 * @brief TODO
		 *
		 * @return true
		 * @return false
		 */
		virtual bool isInitialized() const = 0;

		/**
		 * @brief Calculate the size needed to store the output
		 *        for a provided number of input frames.
		 *
		 * @param inputFrames
		 * @return Size Safe buffer size in frames to store the result of @see process
		 */
		virtual Size calculateBufferSize(Size inputFrames) const = 0;

		// TODO test if this even compiles/works
		/**
		 * @brief Resamples the provided buffer from its sampleRate
		 * to the target rate
		 * @param buffer Audiobuffer to resample, set the rate of the buffer object
		 * @param rateOut Desired output samplerate in Hz
		 * @param quality @see IResamplerTpl
		 */
		static void resample(Buffer& buffer, const Size rateOut, const Size quality = 5) {
			// TODO tklb compensate delay
			const auto rateIn = buffer.sampleRate;
			const Size samples = buffer.size();
			TKLB_ASSERT(rateIn > 0)
			// Make a copy, this could be skipped when a conversion to float is needed anyways
			Buffer copy;
			copy.resize(buffer);
			copy.set(buffer);
			copy.sampleRate = rateIn;
			copy.setValidSize(samples);

			IResamplerTpl resampler;
			resampler.init(rateIn, rateOut, copy.size(), quality);
			buffer.resize(resampler.calculateBufferSize(samples));

			resampler.process(copy, buffer);
		}
	};
}

#endif // _TKLB_IRESAMPLER
