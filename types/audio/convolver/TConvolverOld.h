#ifndef TKLB_CONVOLVER
#define TKLB_CONVOLVER

#include "../../../util/TNoCopy.h"
#include "../../../util/TAssert.h"

#define TKLB_CONVOLUTION_FLOAT

/**
 * Decide which type the convolution will use
 */
#ifndef TKLB_CONVOLUTION_FLOAT
	#define FFTCONVOLVER_TYPE double
	#ifdef TKLB_SAMPLE_FLOAT
		#define TKLB_CONVOLUTION_NEEDS_CONVERSION
	#endif
#else
#define FFTCONVOLVER_TYPE float
	// float convolution can use SSE
	#ifdef TKLB_NO_SIMD
		#define FFTCONVOLVER_DONT_USE_SSE
	#else
		#define FFTCONVOLVER_USE_SSE
	#endif
	#ifndef TKLB_SAMPLE_FLOAT
		#define TKLB_CONVOLUTION_NEEDS_CONVERSION
	#endif
#endif

#include "../../../external/convolver/twoStageConvolver.h"

#include <atomic>

namespace tklb {
/**
 * Wraps up the FttConvolver to do easy stereo convolution
 */
template <unsigned int MAX_BUFFER = 512>
class Convolver {

public:
	using uchar = unsigned char;
	using uint = unsigned int;
#ifdef TKLB_MAXCHANNELS
	static const uchar MAX_CHANNELS = TKLB_MAXCHANNELS;
#else
	static const uchar MAX_CHANNELS = 2;
#endif

private:
	// The convolver maintains internal state so each channels need its own
	fftconvolver::TwoStageFFTConvolver mConvolvers[MAX_CHANNELS];

	std::atomic<bool> mIRLoaded = { false };
	std::atomic<bool> mIsProcessing = { false };

	uint mBlockSize, mTailBlockSize;
	uchar mChannels;

#ifdef TKLB_CONVOLUTION_NEEDS_CONVERSION
#ifdef TKLB_SAMPLE_FLOAT
	using SAMPLE = float;
#else
	using SAMPLE = double;
#endif
	/** Buffers need to be converted from double to float */
	SAMPLE mConversionBufferIn[MAX_BUFFER];
	SAMPLE mConversionBufferOut[MAX_BUFFER];
#endif

public:
	TKLB_NO_COPY(Convolver)

	/**
	 * @param channels How many channels the convolver has. Can't exceed TKLB_MAXCHANNELS
	 * @param block The size of each convolution block
	 * @param tail The size of the tail convolution block
	 */
	Convolver(uchar channels = 2, uint block = 128, uint tail = 4096) {
		mBlockSize = block;
		mTailBlockSize = tail;
		mChannels = channels;
		TKLB_ASSERT(MAX_CHANNELS <= channels);
	}

	void loadIR(
			FFTCONVOLVER_TYPE** samples,
			const unsigned int sampleCount,
			const unsigned int channelCount
	) {
		if (samples == nullptr || sampleCount == 0 || channelCount == 0) { return; }
		mIRLoaded = false;

		while (mIsProcessing) { /** TODO does this even work like a mutex? */ }

		for (uchar c = 0; c < mChannels; c++) {
			mConvolvers[c].init(
				mBlockSize, mTailBlockSize, samples[c % channelCount], sampleCount
			);
		}
		mIRLoaded = true;
	}

	/**
	 * Native process function which does no sample type conversion
	 */
	void process(
			FFTCONVOLVER_TYPE** in, FFTCONVOLVER_TYPE** out, const uint nFrames
	) {
		if (!mIRLoaded) { // just pass the signal through
			for (uchar c = 0; c < mChannels; c++) {
				memcpy(out[c], in[c], sizeof(FFTCONVOLVER_TYPE) * nFrames);
			}
			return;
		}

		mIsProcessing = true;

		for(uchar c = 0; c < mChannels; c++) {
			mConvolvers[c].process(in[c], out[c], nFrames);
		}
		mIsProcessing = false;
	}

#ifdef TKLB_CONVOLUTION_NEEDS_CONVERSION
	/**
	 * Conversion function used when the convolution type
	 * and global sample type don't match
	 */
	void process(SAMPLE** in, SAMPLE** out, const uint nFrames) {
		if (!mIRLoaded) { // just pass the signal through
			for (uchar c = 0; c < mChannels; c++) {
				memcpy(out[c], in[c], sizeof(SAMPLE) * nFrames);
			}
			return;
		}

		mIsProcessing = true;

		for(uchar c = 0; c < mChannels; c++) {
			for (unsigned int i = 0; i < nFrames; i++) {
				mConversionBufferIn[i] = in[c][i];
			}
			mConvolvers[c].process(
				mConversionBufferIn, mConversionBufferOut, nFrames
			);
			for (uint i = 0; i < nFrames; i++) {
				out[c][i] = mConversionBufferOut[i];
			}
		}
		mIsProcessing = false;
	}

	// void loadIR(
	// 		SAMPLE** samples,
	// 		const uint sampleCount,
	// 		const uchar channelCount
	// ) {
	// 	if (samples == nullptr || sampleCount == 0 || channelCount == 0) { return; }
	// 	mIRLoaded = false;

	// 	while (mIsProcessing) { /** TODO does this even work like a mutex? */ }

	// 	for (uchar c = 0; c < mChannels; c++) {
	// 		mConvolvers[c].init(
	// 			mBlockSize, mTailBlockSize, samples[c % channelCount], sampleCount
	// 		);
	// 	}
	// 	mIRLoaded = true;
	// }
#endif

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
