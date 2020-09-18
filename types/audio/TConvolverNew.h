#ifndef TKLB_CONVOLVER
#define TKLB_CONVOLVER

#include "../../util/TAssert.h"
#include "../../util/TMath.h"
#include <cmath>
#include "./TAudioBuffer.h"
#include "./fft/TFFT.h"

#ifndef TKLB_NO_SIMD
	#include "../../external/xsimd/include/xsimd/xsimd.hpp"
#endif

namespace tklb {


template <typename T>
class SingleConvolverTpl {
public:
	using uchar = unsigned char;
	using uint = unsigned int;
	using Buffer = AudioBufferTpl<T>;

private:
	uint mBlockSize; // Power of 2 blocksize
	uint mSegmentSize; // Double the block size, for overlap?
	uint mSegmentCount; // number of blocks that fit in the IR length
	uint mFFTComplexSize; // Blocksize + 1
	uint mInputBufferFill; // How much is buffered
	uint mCurrentPosition; // segment index
	Buffer mComplexIr; // FFT of the impulse response
	Buffer mFFTBuffer; // space for the FFT of the input signal

	using Segments = std::vector<Buffer>;
	Segments mSegmentsIR; // FFT of the IR in segments
	Segments mSegments; // ?
	FFT mFFT; // FFT object, fixed type and will do conversions when needed
	Buffer mPremultipliedBuffer;
	Buffer mInputBuffer;
	Buffer mConvolutionBuffer;
	Buffer mOverlapBuffer;

public:
	template <typename T2>
	void load(const AudioBufferTpl<T2>& buffer, const uint blockSize) {
		uint irLength = buffer.size();
		const T2* ir = buffer[0];
		// trim silence, since longer IR increase CPU usage considerably
		const T2 silence = 0.000001;
		while (--irLength && silence > fabs(ir[irLength])) { }
		if (irLength == 0) { return; }

		// Figure out how many segments a block is
		mBlockSize = powerOf2(blockSize);
		mSegmentSize = 2 * mBlockSize;
		mSegmentCount = std::ceil(irLength / double(mBlockSize));
		mFFTComplexSize = mBlockSize + 1;

		// create segment buffers for the input signal
		mFFT.resize(mSegmentSize);
		mFFTBuffer.resize(mSegmentSize, 1);

		// create segment buffers for the impulse response
		mSegments.resize(mSegmentCount);
		mSegmentsIR.resize(mSegmentCount);

		for (uint i = 0; i < mSegmentCount; i++) {
			const uint remaining = std::min(irLength - (i * mBlockSize), mBlockSize);
			mFFTBuffer.set(0);
			// Put the segment in the fft buffer, might do type conversion
			mFFTBuffer.set(ir + (i * mBlockSize), 0, remaining);
			mSegmentsIR[i].resize(mFFTComplexSize, 2); // make space in the result buffer
			mSegments[i].resize(mFFTComplexSize, 2); // and the input signal buffer
			mFFT.forward(mFFTBuffer, mSegmentsIR[i]); // save the fft result for each segment
		}

		mPremultipliedBuffer.resize(mFFTComplexSize, 2);
		mConvolutionBuffer.resize(mFFTComplexSize, 2);
		mOverlapBuffer.resize(mBlockSize, 1);
		mInputBuffer.resize(mBlockSize, 1);

		mCurrentPosition = 0;
		mInputBufferFill = 0;
	}

	template <typename T2>
	void process(const AudioBufferTpl<T2>& inBuf, AudioBufferTpl<T2>& outBuf) {
		const uint length = inBuf.validSize();
		const T2* in = inBuf[0];
		T2* out = outBuf[0];
		uint processed = 0;

		while (processed < length) {
			const bool inputBufferWasEmpty = mInputBufferFill == 0;
			const uint processing = std::min(length - processed, mBlockSize - mInputBufferFill);
			const uint inputBufferPos = mInputBufferFill;

			mInputBuffer.set(in + processed, inputBufferPos, processing);
			mFFTBuffer.set(0);
			mFFT.forward(mFFTBuffer, mSegments[mCurrentPosition]);

			if (inputBufferWasEmpty) {
				mPremultipliedBuffer.set(0);
				for (uint i = 0; i < mSegmentCount; i++) {
					const size_t indexIr = i;
					const size_t indexAudio = (mCurrentPosition + i) % mSegmentCount;
					complexMultiply(mPremultipliedBuffer, mSegmentsIR[indexIr], mSegments[indexAudio]);
				}
			}

			mConvolutionBuffer.set(mPremultipliedBuffer);

			complexMultiply(mConvolutionBuffer, mSegments[mCurrentPosition], mSegmentsIR[0]);

			mFFT.back(mConvolutionBuffer, mFFTBuffer);


			mInputBufferFill += processing;
			if (mInputBufferFill == mBlockSize) {
				mInputBuffer.set(0);
				mInputBufferFill = 0;
				mOverlapBuffer.set(mFFTBuffer[0] + mBlockSize, 0, mBlockSize);
				mCurrentPosition = (mCurrentPosition > 0) ? (mCurrentPosition - 1) : (mSegmentCount - 1);
			}

			mFFTBuffer.add(mOverlapBuffer, 0, processing);
			mFFTBuffer.set(out + processed, 0, processing, 0);
			processed += processing;
		}
	}

private:
	static void complexMultiply(
		const Buffer& bufferA, const Buffer& bufferB, Buffer& bufferOut
	) {
		const uint size = bufferOut.size();
		const T* aReal = bufferA.get(0);
		const T* aImag = bufferA.get(1);
		const T* bReal = bufferB.get(0);
		const T* bImag = bufferB.get(1);
		T* outReal = bufferOut.get(0);
		T* outImag = bufferOut.get(1);
		#ifdef TKLB_NO_SIMD
			for (uint i = 0; i < size; i++) {
				outReal[i] += aReal[i] * bReal[i] - aImag[i] * bImag[i];
				outImag[i] += aReal[i] * bImag[i] + aImag[i] * bReal[i];
			}
		#else
			const uint vectorize = size - size % Buffer::stride;
			for(uint i = 0; i < vectorize; i += Buffer::stride) {
				xsimd::simd_type<T> aR = xsimd::load_aligned(aReal + i);
				xsimd::simd_type<T> bR = xsimd::load_aligned(bReal + i);
				xsimd::simd_type<T> aI = xsimd::load_aligned(aImag + i);
				xsimd::simd_type<T> bI = xsimd::load_aligned(bImag + i);
				xsimd::store_aligned(outReal + i, (aR * bR) - (aI * bI));
				xsimd::store_aligned(outImag + i, (aR * bI) + (aI * bR));
			}
			for(uint i = vectorize; i < size; i++) {
				outReal[i] += aReal[i] * bReal[i] - aImag[i] * bImag[i];
				outImag[i] += aReal[i] * bImag[i] + aImag[i] * bReal[i];
			}
		#endif
	}

};

typedef SingleConvolverTpl<float> SingleConvolverFloat;
typedef SingleConvolverTpl<double> SingleConvolverDouble;

// Default type
#ifdef TKLB_SAMPLE_FLOAT
	typedef SingleConvolverFloat SingleConvolver;
#else
	typedef SingleConvolverDouble SingleConvolver;
#endif

} // namespace

#endif // TKLB_CONVOLVER
