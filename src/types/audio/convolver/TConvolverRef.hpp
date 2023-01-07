#ifndef _TKLB_CONVOLVER_REF
#define _TKLB_CONVOLVER_REF

#include "../../../util/TAssert.h"
#include "../TAudioBuffer.hpp"

#ifdef TKLB_NO_SIMD
	#define FFTCONVOLVER_DONT_USE_SSE
#else
	#define FFTCONVOLVER_USE_SSE
#endif

#include "../../../../external/convolver/twoStageConvolver.h"

namespace tklb {
	/**
	 * Wraps up the FttConvolver to support type conversion
	 */
	template <typename T>
	class ConvolverRefTpl {
		using Buffer = AudioBufferTpl<T>;
		using uchar = unsigned char;
		using Size = typename Buffer::Size;

		fftconvolver::FFTConvolver mConvolvers[AudioBufferTpl<T>::MAX_CHANNELS];
		// IN case conversion to internal sample type is needed
		AudioBufferTpl<fftconvolver::Sample> mConversion;
		Size mBlockSize;
		uchar mIrChannels = 0;

	public:
		ConvolverRefTpl() = default;

		/**
		 * @brief Load a impulse response and prepare the convolution
		 * @param buffer The ir buffer.
		 * @param channel Which channel to use from the AudioBuffer
		 * @param blockSize Size of blocks ir will be divided in
		 */
		template <typename T2>
		void load(const AudioBufferTpl<T2>& ir, const Size blockSize) {
			// trim silence, since longer IRs increase CPU usage considerably
			Size irLength = ir.size();
			if (irLength == 0) { return; }
			const T2 silence = 0.0001;

			for (Size i = irLength - 1; 0 < i; i--) {
				for (uchar c = 0; c < ir.channels(); c++) {
					if (silence < fabs(ir[c][i])) {
						// if any of the channels are over the
						// silence threshold we use the current
						// size and bail
						irLength = i + 1;
						goto endLoop;
					}
				}
			}
		endLoop:
			mIrChannels = ir.channels();
			mBlockSize = blockSize;

			if (std::is_same<T2, fftconvolver::Sample>::value) {
				for (uchar c = 0; c < mIrChannels; c++) {
					auto buffer = reinterpret_cast<const fftconvolver::Sample*>(ir[c]);
					mConvolvers[c].init(blockSize, buffer, irLength);
				}
			} else {
				mConversion.clone(ir);
				for (uchar c = 0; c < mIrChannels; c++) {
					mConvolvers[c].init(blockSize, mConversion[c], irLength);
				}
			}

			mConversion.resize(blockSize); // in case we need to convert
		}

		/**
		 * @brief Do the convolution
		 * @param in Input signal, can be mono
		 * @param out Output buffer, needs to have enough space allocated
		 */
		template <typename T2>
		void process(const AudioBufferTpl<T2>& in, AudioBufferTpl<T2>& out) {
			const Size length = in.validSize();
			const Size n = out.size();
			Size samplesLeft = n;

			for (Size i = 0; i < length; i += mBlockSize) {
				const Size remaining = tklb::min(mBlockSize, samplesLeft);
				for (uchar c = 0; c < out.channels(); c++) {
					// eg the input is mono, but the IR stereo
					// the result will still be stereo
					const uchar inChannel = c % in.channels();
					// TODO tklb the convolver channels can't be crossed
					// so each output needds its own convolver
					if (tklb::traits::IsSame<T2, fftconvolver::Sample>::value) {
						auto inBuf = reinterpret_cast<const fftconvolver::Sample*>(in[inChannel] + i);
						auto outBuf = reinterpret_cast<fftconvolver::Sample*>(out[c] + i);
						mConvolvers[c].process(inBuf, outBuf, remaining);
					} else {
						// TODO tklb conversion
					}

				}
				samplesLeft -= remaining;
			}
		}

		static const char* getLicense() {
			return
				"Realtime Convolution by\n"
				"https://github.com/HiFi-LoFi\n"
				"https://github.com/HiFi-LoFi/FFTConvolver\n"
				"MIT License\n\n";
		}
	};

	using ConvolverRefFloat = ConvolverRefTpl<float>;
	using ConvolverRefDouble = ConvolverRefTpl<double>;

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ConvolverRef = ConvolverRefTpl<float>;
	#else
		using ConvolverRef = ConvolverRefTpl<double>;
	#endif
} // namespace

#endif // _TKLB_CONVOLVER_REF
