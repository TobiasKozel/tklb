#ifndef TKLB_FFT
#define TKLB_FFT


#include <vector>
#include "../TAudioBuffer.h"
#include "../../../external/Ouura.h"

namespace tklb {

class FFT {
	using uchar = unsigned char;
	using uint = unsigned int;

	uint mSize;
	// No idea what this is
	AudioBuffer<>::Buffer<int> mIp;
	// or this, prolly lookup tables
	AudioBuffer<double> mW = { 1 };
	AudioBuffer<double> mBuffer { 1 };

public:

	FFT(uint size = 0) {
		if (size == 0) { return; }
		resize(size);
	}

	void resize(uint size) {
		if (size == mSize) { return; }
		mIp.resize(
			2 + static_cast<int>(std::sqrt(static_cast<double>(size)))
		);
		mW.resize(size / 2);
		mBuffer.resize(size);
		mSize = size;

		const int size4 = static_cast<int>(size) / 4;
		ooura::makewt(size4, mIp.data(), mW.get(0));
		ooura::makect(size4, mIp.data(), mW.get(0) + size4);
	}

	template <typename T>
	void forward(const AudioBuffer<T>& input, AudioBuffer<double>& result) {
		double* real = result.get(0);
		double* imaginary = result.get(1);
		/**
		 * converts or copies the data
		 * we shouldn't use the original buffer since
		 * mBuffer gets written toby the fft
		 */
		mBuffer.set(input);
		ooura::rdft(mSize, +1, mBuffer.get(0), mIp.data(), mW.get(0));

		// Convert back to split-complex
		{
			double* b = mBuffer.get(0);
			double* bEnd = b + mSize;
			double* r = real;
			double* i = imaginary;
			while (b != bEnd) {
				*(r++) = (*(b++));
				*(i++) = (-(*(b++)));
			}
		}
		const size_t size2 = mSize / 2;
		real[size2] = -imaginary[0];
		imaginary[0] = 0.0;
		imaginary[size2] = 0.0;
	}

	template <typename T>
	void back(const AudioBuffer<double>& result, AudioBuffer<T>& input) {
		T* data = input.get(0);
		const double* real = result.get(0);
		const double* imaginary = result.get(1);
		{
			double* b = mBuffer.get(0);
			double* bEnd = b + mSize;
			const double* r = real;
			const double* i = imaginary;
			while (b != bEnd) {
				*(b++) = static_cast<double>(*(r++));
				*(b++) = -static_cast<double>(*(i++));
			}
			mBuffer[0][1] = real[mSize / 2];
		}
		ooura::rdft(static_cast<int>(mSize), -1, mBuffer.get(0), mIp.data(), mW.get(0));

		const T volume = 2.0 / static_cast<double>(mSize);
		if (std::is_same<T, double>::value) {
			mBuffer.multiply(volume); // scale the output
			mBuffer.put(reinterpret_cast<double*>(data), 0, mSize);
		} else {
			// or the old fashioned way if we need to convert sample types anyways
			const double* buf = mBuffer.get(0);
			for (uint i = 0; i < mSize; i++) {
				data[i] = buf[i] * volume;
			}
		}
	}
};

} // namespace

#endif // TKLB_FFT
