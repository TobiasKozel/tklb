#ifndef TKLBZ_RESAMPLER_SPEEX
#define TKLBZ_RESAMPLER_SPEEX

#include "../../../util/TAssert.h"
#include "../../../memory/TMemory.hpp"
#include "../TAudioBuffer.hpp"

#define FLOATING_POINT

#ifndef TKLB_NO_SIMD
	#ifdef __arm__
		#define USE_NEON
	#else
		#define USE_SSE
		#define USE_SSE2
	#endif
#endif

#define OUTSIDE_SPEEX
#define RANDOM_PREFIX tklb

static inline void* speex_alloc (int size) {
	void* ptr = TKLB_MALLOC(size);
	::tklb::memory::set(ptr, 0, size);
	return ptr;
}

static inline void speex_free (void* ptr) {
	TKLB_FREE(ptr);
}

static inline void* speex_realloc (void* ptr, int size) {
	return TKLB_REALLOC(ptr, size);
}

#include "../../../../external/speex_resampler/speex_resampler.h"

#ifdef TKLBZ_RESAMPLER_IMPL
	#include "./TResamplerSpeex.cpp"
#endif

namespace tklb {

	template <typename T>
	class ResamplerSpeexTpl {
		using uchar = unsigned char;
		using uint = unsigned int;
		using Buffer = AudioBufferTpl<T>;
		using Size = typename Buffer::Size;

		uint mRateIn, mRateOut;
		AudioBufferFloat mConvertOut, mConvertIn;
		static constexpr uchar MAX_CHANNELS = AudioBufferFloat::MAX_CHANNELS;
		SpeexResamplerState* mState = nullptr;

	public:
		ResamplerSpeexTpl(uint rateIn, uint rateOut, uint maxBlock = 512, uchar quality = 5) {
			init(rateIn, rateOut, maxBlock, quality);
		}

		ResamplerSpeexTpl() = default;

		~ResamplerSpeexTpl() {
			speex_resampler_destroy(mState);
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
			int err;
			if (mState != nullptr) {
				speex_resampler_destroy(mState);
			}
			mState = speex_resampler_init(MAX_CHANNELS, rateIn, rateOut, quality, &err);

			// Conversion buffers if not doing float resampling
			if (!std::is_same<T, float>::value) {
				mConvertIn.resize(maxBlock, MAX_CHANNELS);
				mConvertOut.resize(calculateBufferSize(rateIn, rateOut, maxBlock), MAX_CHANNELS);
			}
			mRateIn = rateIn;
			mRateOut = rateOut;
			return err == 0;
		}

		/**
		 * @brief Resample function
		 * Make sure the out buffer has enough space
		 */
		Size process(const AudioBufferTpl<T>& in, AudioBufferTpl<T>& out) {
			TKLB_ASSERT(in.sampleRate == mRateIn);
			TKLB_ASSERT(out.sampleRate == mRateOut);
			TKLB_ASSERT(in.validSize() > 0)
			Size samplesOut = 0;
			if (std::is_same<T, float>::value) {
				// Input output buffer must not overlap when working directly on them
				TKLB_ASSERT(&in != &out)
				for (uchar c = 0; c < in.channels(); c++) {
					Size countIn = in.validSize();
					Size countOut = out.size();
					const float* inBuf = reinterpret_cast<const float*>(in[c]);
					float* outBuf = reinterpret_cast<float*>(out[c]);
					speex_resampler_process_float(mState, c, inBuf, &countIn, outBuf, &countOut);
					samplesOut = countOut;
				}
			} else {
				const Size validSamples = in.validSize();
				const Size blockSize = mConvertIn.size();
				for (Size i = 0; i < validSamples; i += blockSize) {
					const Size blockLeft = min(blockSize, validSamples - i);
					Size samplesEmitted = 0;
					mConvertIn.set(in, blockLeft, i);
					for (uchar c = 0; c < in.channels(); c++) {
						spx_uint32_t countIn = blockLeft;
						spx_uint32_t countOut = mConvertOut.size();
						const float* inBuf = mConvertIn[c];
						float* outBuf = mConvertOut[c];
						speex_resampler_process_float(mState, c, inBuf, &countIn, outBuf, &countOut);
						samplesEmitted = countOut;
						TKLB_ASSERT(mConvertOut.size() >= countOut);
					}
					out.set(mConvertOut, samplesEmitted, 0, samplesOut);
					samplesOut += samplesEmitted;
				}
			}

			// There might be some more samples beeing emitted, some padding is needed
			TKLB_ASSERT(out.size() >= samplesOut);
			out.setValidSize(samplesOut);
			return samplesOut;
		}

		/**
		 * @brief Get the latency in samples
		 */
		int getLatency() const {
			return speex_resampler_get_input_latency(mState);
		}

		/**
		 * @brief Estimate how many samples need to be put in to get n samples out.
		 */
		Size estimateNeed(const Size in) {
			return in * (mRateIn / double(mRateOut));
		}

		bool isInitialized() const { return mState != nullptr; }

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
			const Size samples = buffer.size();
			TKLB_ASSERT(rateIn > 0)
			// Make a copy, this could be skipped when a conversion to float is needed anyways
			Buffer copy;
			copy.resize(buffer);
			copy.set(buffer);
			copy.sampleRate = rateIn;
			copy.setValidSize(samples);

			buffer.resize(calculateBufferSize(rateIn, rateOut, samples));

			ResamplerSpeexTpl<T> resampler;
			resampler.init(rateIn, rateOut, copy.size(), quality);
			resampler.process(copy, buffer);
		}

	};

	using ResamplerSpeexFloat = ResamplerSpeexTpl<float>;
	using ResamplerSpeexDouble = ResamplerSpeexTpl<double>;

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ResamplerSpeex = ResamplerSpeexTpl<float>;
	#else
		using ResamplerSpeex = ResamplerSpeexTpl<double>;
	#endif

} // namespace

#endif // RESAMPLER