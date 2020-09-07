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
	uint mBlockSize, mSegmentSize, mSegmentCount, mFFTSize;
	AudioBuffer<> mComplexIr, mFFTBuffer;
	std::vector<AudioBuffer<>> mSegments, mSegmentsIR;
	FFT mFFT;

public:
	// template <typename T>
	using T = double;
	void load(const AudioBuffer<>::Buffer<T>& ir, const uint blockSize) {
		// reset
		uint length = ir.size();

		// trim silence
		const T silence = 0.000001;
		while (length-- && silence < fabs(ir[length])) { }

		mBlockSize = powerOf2(blockSize);
		mSegmentSize = 2 * mBlockSize;
		mSegmentCount = std::ceil(length / T(mBlockSize));
		mFFTSize = (mSegmentSize / 2) + 1;

		mFFT.resize(mSegmentSize);
		mFFTBuffer.resize(mSegmentSize, 1);
	}

};

} // namespace

#endif // TKLB_CONVOLVER
