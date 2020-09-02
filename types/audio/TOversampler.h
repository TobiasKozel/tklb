#ifndef TKLB_OVERSAMPLER
#define TKLB_OVERSAMPLER

#include <functional>
#include "../../util/TNoCopy.h"
#include "../../util/TAssert.h"
#include "../TPointerList.h"
#include "./TAudioBuffer.h"

#ifdef TKLB_SAMPLE_FLOAT
	#define TKLB_OVERSAMPLE_FLOAT
#endif

#ifndef TKLB_NO_SIMD
	#ifdef __arm__
		#ifdef TKLB_OVERSAMPLE_FLOAT
			// TODO PERF maybe try Upsampler2x4Neon instead
			#include "../../external/hiir/hiir/Upsampler2xNeon.h"
			#include "../../external/hiir/hiir/Downsampler2xNeon.h"
			#define TKLB_OVERSAMPLER_UP(coeffs) hiir::Upsampler2xNeon<coeffs>
			#define TKLB_OVERSAMPLER_DOWN(coeffs) hiir::Downsampler2xNeon<coeffs>
		#else
			// Software implementation, there doesn't seem to be a double simd version
			#include "../../external/hiir/hiir/Upsampler2xFpu.h"
			#include "../../external/hiir/hiir/Downsampler2xFpu.h"
			#define TKLB_OVERSAMPLER_UP(coeffs) hiir::Upsampler2xFpuTpl<coeffs, double>
			#define TKLB_OVERSAMPLER_DOWN(coeffs) hiir::Downsampler2xFpuTpl<coeffs, double>
		#endif
	#else
		// TODO PERF maybe try avx
	#ifdef TKLB_OVERSAMPLE_FLOAT
		// #include "../../external/hiir/hiir/Upsampler2x8Avx.h"
		// #include "../../external/hiir/hiir/Downsampler2x8Avx.h"
		// #define TKLB_OVERSAMPLER_UP(coeffs) hiir::Upsampler2x8Avx<coeffs>
		// #define TKLB_OVERSAMPLER_DOWN(coeffs) hiir::Downsampler2x8Avx<coeffs>
		#include "../../external/hiir/hiir/Upsampler2xSse.h"
		#include "../../external/hiir/hiir/Downsampler2xSse.h"
		#define TKLB_OVERSAMPLER_UP(coeffs) hiir::Upsampler2xSse<coeffs>
		#define TKLB_OVERSAMPLER_DOWN(coeffs) hiir::Downsampler2xSse<coeffs>
	#else
		// #include "../../external/hiir/hiir/Upsampler2x4F64Avx.h"
		// #include "../../external/hiir/hiir/Downsampler2x4F64Avx.h"
		// #define TKLB_OVERSAMPLER_UP(coeffs) hiir::Upsampler2x4F64Avx<coeffs>
		// #define TKLB_OVERSAMPLER_DOWN(coeffs) hiir::Downsampler2x4F64Avx<coeffs>
		#include "../../external/hiir/hiir/Upsampler2xF64Sse2.h"
		#include "../../external/hiir/hiir/Downsampler2xF64Sse2.h"
		#define TKLB_OVERSAMPLER_UP(coeffs) hiir::Upsampler2xF64Sse2<coeffs>
		#define TKLB_OVERSAMPLER_DOWN(coeffs) hiir::Downsampler2xF64Sse2<coeffs>
	#endif
	#endif
#else
	// Software implementation
	#include "../../external/hiir/hiir/Upsampler2xFpu.h"
	#include "../../external/hiir/hiir/Downsampler2xFpu.h"
	#define TKLB_OVERSAMPLER_UP(coeffs) hiir::Upsampler2xFpuTpl<coeffs, T>
	#define TKLB_OVERSAMPLER_DOWN(coeffs) hiir::Downsampler2xFpuTpl<coeffs, T>
#endif

namespace tklb {
/**
 * Super simple wrapper for the hiir up and down samplers
 * It only goes up to 4x oversampling
 */
#ifdef TKLB_MAXCHANNELS
template <int CHANNELS = TKLB_MAXCHANNELS, int MAX_BLOCK = 512>
#else
template <int CHANNELS = 2, int MAX_BLOCK = 512>
#endif
class Oversampler {
public:
	using T = AudioBuffer::T;

	using uchar = unsigned char;
	using uint = unsigned int;

	using ProcessFunction = std::function<void(T**, T**, uint)>;

private:
	TKLB_OVERSAMPLER_UP(12) mUp2x[CHANNELS];
	TKLB_OVERSAMPLER_UP(4) mUp4x[CHANNELS];
	TKLB_OVERSAMPLER_DOWN(12) mDown2x[CHANNELS];
	TKLB_OVERSAMPLER_DOWN(4) mDown4x[CHANNELS];

	AudioBuffer mBuf2xUp, mBuf2xDown, mBuf4xUp, mBuf4xDown;

	/**
	 * Straight up stolen from the hiir oversampler wrapper from iPlug2
	 * https://github.com/iPlug2/iPlug2/blob/master/IPlug/Extras/Oversampler.h
	 */
	const double coeffs2x[12] = {
		0.036681502163648017, 0.13654762463195794, 0.27463175937945444,
		0.42313861743656711, 0.56109869787919531, 0.67754004997416184,
		0.76974183386322703, 0.83988962484963892, 0.89226081800387902,
		0.9315419599631839, 0.96209454837808417, 0.98781637073289585
	};
	const double coeffs4x[4] = {
		0.041893991997656171, 0.16890348243995201,
		0.39056077292116603, 0.74389574826847926
	};

	uchar mFactor = 1;
	ProcessFunction mProc;
public:
	TKLB_NO_COPY(Oversampler)

	Oversampler() {
		for (uchar c = 0; c < CHANNELS; c++) {
			mUp2x[c].set_coefs(coeffs2x);
			mDown2x[c].set_coefs(coeffs2x);
			mUp4x[c].set_coefs(coeffs4x);
			mDown4x[c].set_coefs(coeffs4x);
		}
		mBuf2xDown.resize(MAX_BLOCK * 2, CHANNELS);
		mBuf2xUp.resize(MAX_BLOCK * 2, CHANNELS);
		mBuf4xDown.resize(MAX_BLOCK * 4, CHANNELS);
		mBuf4xUp.resize(MAX_BLOCK * 4, CHANNELS);
	}

	void setProcessFunc(const ProcessFunction& f) {
		mProc = f;
	}

	void setFactor(const uchar factor) {
		mFactor = factor;
	}

	int getFactor() const {
		return mFactor;
	}

	void process(AudioBuffer& in, AudioBuffer& out) {
		process(in.getRaw(), out.getRaw(), in.size());
	}

	void process(T** in, T** out, const uint frames) {
		/**
		 * No OverSampling at all
		 */
		if (mFactor == 1) {
			mProc(in, out, frames);
			return;
		}

		/**
		 * 2x OverSampling
		 */
		if (mFactor == 2 || mFactor == 3) {
			for (uchar c = 0; c < CHANNELS; c++) {
				mUp2x[c].process_block(mBuf2xUp[c], in[c], frames);
			}

			mProc(mBuf2xUp.getRaw(), mBuf2xDown.getRaw(), frames * 2);

			for (uchar c = 0; c < CHANNELS; c++) {
				mDown2x[c].process_block(out[c], mBuf2xDown[c], frames);
			}
			return;
		}

		/**
		 * 4x OverSampling
		 */
		for (uchar c = 0; c < CHANNELS; c++) {
			mUp2x[c].process_block(mBuf2xUp[c], in[c], frames);
			mUp4x[c].process_block(mBuf4xUp[c], mBuf2xUp[c], frames * 2);
		}

		mProc(mBuf4xUp.getRaw(), mBuf4xDown.getRaw(), frames * 4);

		for (uchar c = 0; c < CHANNELS; c++) {
			mDown4x[c].process_block(mBuf2xDown[c], mBuf4xDown[c], frames * 2);
			mDown2x[c].process_block(out[c], mBuf2xDown[c], frames);
		}
	}

	static const char* getLicense() {
		return
			"HIIR Library by\n"
			"WTFPL Licensed";
	}
};

} // namespace

#endif // TKLB_OVERSAMPLER
