#ifndef TKLB_CONVOLVER
#define TKLB_CONVOLVER

#include "../../util/NoCopy.h"

/**
 * Decide which type the convolution will use
 */
#ifndef TKLB_FLOAT_CONVOLUTION
	#define FFTCONVOLVER_TYPE double
#else
#define FFTCONVOLVER_TYPE float
	// float convolution can use SSE
	#ifdef TKLB_INTRINSICS
		#define FFTCONVOLVER_USE_SSE
	#endif
#endif

#include "../../external/convolver/twoStageConvolver.h"

#include <atomic>

namespace tklb {
/**
 * Wraps up the FttConvolver to do easy stereo convolution
 * and also deal with the buffers
 */
template <int CHANNELS = 2>
class Convolver {
	// The convolver maintains internal state so each channels need its own
	fftconvolver::TwoStageFFTConvolver mConvolvers[CHANNELS];

	std::atomic<bool> mIRLoaded = { false };
	std::atomic<bool> mIsProcessing = { false };

	int mBlockSize, mTailBlockSize;

public:
	TKLB_NO_COPY(Convolver)

	explicit Convolver(int block = 128, int tail = 4096){
		mBlockSize = block;
		mTailBlockSize = tail;
	}

	void loadIR(float** samples, const size_t sampleCount, const size_t channelCount) {
		if (samples == nullptr || sampleCount == 0 || channelCount == 0) { return; }
		mIRLoaded = false;

		while (mIsProcessing) { /** TODO does this even work like a mutex? */ }

		for (int c = 0; c < CHANNELS; c++) {
			mConvolvers[c].init(
				mBlockSize, mTailBlockSize, samples[c % channelCount], sampleCount
			);
		}
		mIRLoaded = true;
	}

	void ProcessBlock(FFTCONVOLVER_TYPE** in, FFTCONVOLVER_TYPE** out, const int nFrames) {
		if (!mIRLoaded) { // just pass the signal through
			for (int c = 0; c < CHANNELS; c++) {
				for (int i = 0; i < nFrames; i++) {
					out[c][i] = in[c][i];
				}
			}
			return;
		}

		mIsProcessing = true;

		for(int c = 0; c < CHANNELS; c++) {
			mConvolvers[c].process(in[c], out[c], nFrames);
		}
		mIsProcessing = false;
	}

	static const char* getLicense() {
		return
			"Realtime Convolution by\n"
			"https://github.com/HiFi-LoFi\n"
			"https://github.com/HiFi-LoFi/FFTConvolver\n"
			"MIT License\n\n";
	}
};

} // namespace

#endif // TKLB_CONVOLVER