#ifndef TKLB_CONVOLVER
#define TKLB_CONVOLVER

#include "../../util/TNoCopy.h"

/**
 * Decide which type the convolution will use
 */
#ifndef TKLB_CONVOLUTION_FLOAT
	#define FFTCONVOLVER_TYPE double
#else
#define FFTCONVOLVER_TYPE float
	// float convolution can use SSE
	#ifndef TKLB_NO_INTRINSICS
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
class Convolver {
	// The convolver maintains internal state so each channels need its own
	fftconvolver::TwoStageFFTConvolver* mConvolvers[16] = { nullptr };

	std::atomic<bool> mIRLoaded = { false };
	std::atomic<bool> mIsProcessing = { false };

	unsigned int mBlockSize, mTailBlockSize, mChannels;

public:
	TKLB_NO_COPY(Convolver)

	explicit Convolver(int channels = 2, int block = 128, int tail = 4096) {
		mBlockSize = block;
		mTailBlockSize = tail;
		mChannels = channels;
		for (int c = 0; c < channels; c++) {
			mConvolvers[c] = new fftconvolver::TwoStageFFTConvolver();
		}
	}

	void loadIR(const FFTCONVOLVER_TYPE** samples, const size_t sampleCount, const size_t channelCount) {
		if (samples == nullptr || sampleCount == 0 || channelCount == 0) { return; }
		mIRLoaded = false;

		while (mIsProcessing) { /** TODO does this even work like a mutex? */ }

		for (int c = 0; c < mChannels; c++) {
			mConvolvers[c]->init(
				mBlockSize, mTailBlockSize, samples[c % channelCount], sampleCount
			);
		}
		mIRLoaded = true;
	}

	void ProcessBlock(FFTCONVOLVER_TYPE** in, FFTCONVOLVER_TYPE** out, const int nFrames) {
		if (!mIRLoaded) { // just pass the signal through
			for (int c = 0; c < mChannels; c++) {
				for (int i = 0; i < nFrames; i++) {
					out[c][i] = in[c][i];
				}
			}
			return;
		}

		mIsProcessing = true;

		for(int c = 0; c < mChannels; c++) {
			mConvolvers[c]->process(in[c], out[c], nFrames);
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
