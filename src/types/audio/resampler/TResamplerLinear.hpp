#ifndef TKLBZ_RESAMPLER_LINEAR
#define TKLBZ_RESAMPLER_LINEAR

#include "../TAudioBuffer.hpp"

namespace tklb {
	template <typename T>
	class ResamplerLinearTpl {
		using uchar = unsigned char;
		using uint = unsigned int;
		using Buffer = AudioBufferTpl<T>;

		uint mRateIn, mRateOut;
		Buffer mBuffer;
	public:
		ResamplerLinearTpl(uint rateIn, uint rateOut, uint maxBlock = 512, uchar quality = 5) {
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
		uint process(const Buffer& in, Buffer& out) {
			mBuffer.set(in);
			if (mRateIn < mRateOut) {
				// TODO tklb lerp
				// TODO tklb lowpass
			} else {
				// TODO tklb lowpass
				// TODO tklb lerp
			}
			TKLB_ASSERT(false) // Not implemented
		}

		/**
		 * @brief Get the latency in samples
		 */
		uint getLatency() const {
			return 1;
		};

		bool isInitialized() const {
			return true;
		};

		/**
		 * @brief Calculate a buffersize fit for the resampled result.
		 * Also adds a bit of padding.
		 */
		static uint calculateBufferSize(uint rateIn, uint rateOut, uint initialSize) {
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
			const uint samples = buffer.size();
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

	using ResamplerLinearFloat = ResamplerLinearTpl<float>;
	using ResamplerLinearDouble = ResamplerLinearTpl<double>;

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ResamplerLinear = ResamplerLinearTpl<float>;
	#else
		using ResamplerLinear = ResamplerLinearTpl<double>;
	#endif

} // namespace

#endif // TKLBZ_RESAMPLER_LINEAR
