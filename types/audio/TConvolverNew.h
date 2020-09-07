#ifndef TKLB_CONVOLVER
#define TKLB_CONVOLVER

#include "../../util/TAssert.h"
#include "../../util/TMath.h"
#include <cmath>
#include "./TAudioBuffer.h"
#include "./fft/TFFT.h"

namespace tklb {

class SingleConvolver {
public:
	using uchar = unsigned char;
	using uint = unsigned int;

private:
	uint mBlockSize, mSegmentSize, mSegmentCount, mFFTComplexSize;
	uint mInputBufferFill, mCurrentPosition;
	AudioBuffer<> mComplexIr, mFFTBuffer;
	std::vector<AudioBuffer<>> mSegments, mSegmentsIR;
	FFT mFFT;

public:
	// template <typename T>
	using T = double;
	void load(const T* ir, uint irLength, const uint blockSize) {

		// trim silence, since longer IR increase CPU usage considerably
		const T silence = 0.000001;
		while (--irLength && silence > fabs(ir[irLength])) { }
		if (irLength == 0) { return; }

		// Figure out how many segments a block is
		mBlockSize = powerOf2(blockSize);
		mSegmentSize = 2 * mBlockSize;
		mSegmentCount = std::ceil(irLength / T(mBlockSize));
		mFFTComplexSize = (mSegmentSize / 2) + 1;

		// create segment buffers for the input signal
		mSegments.resize(mSegmentCount);
		mFFT.resize(mSegmentSize);
		mFFTBuffer.resize(mSegmentSize, 1);

		// create segment buffers for the impulse response
		mSegmentsIR.resize(mSegmentCount);
		for (uint i = 0; i < mSegmentCount; i++) {
			const uint remaining = std::min(irLength - (i * mBlockSize), mBlockSize);
			mFFTBuffer.set(0);
			// Put the segment in the fft buffer
			mFFTBuffer.set(ir + (i * mBlockSize), 0, remaining);
			mSegmentsIR[i].resize(mFFTComplexSize, 2); // make space in the result buffer
			mSegments[i].resize(mFFTComplexSize, 2); // and the input signal buffer
			mFFT.forward(mFFTBuffer, mSegmentsIR[i]); // save the fft result for each segment
		}

		mCurrentPosition = 0;
		mInputBufferFill = 0;
	}

	void process() {

	}

};

} // namespace

#endif // TKLB_CONVOLVER
