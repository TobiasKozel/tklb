#ifndef TKLB_FFT_OOURA
#define TKLB_FFT_OOURA


#include <vector>
#include "../TAudioBuffer.h"
#include "../../../external/Ouura.h"

namespace tklb {

class FFTOoura {
	using uchar = unsigned char;
	using uint = unsigned int;

	uint mSize;
	// No idea what this is
	AudioBuffer::Buffer<int> mIp;
	// or this, prolly lookup tables
	AudioBufferDouble mW = { 1 };
	AudioBufferDouble mBuffer { 1 };

public:

	FFTOoura(uint size = 0) {
		if (size == 0) { return; }
		resize(size);
	}

	void resize(uint size) {
		if (size == mSize) { return; }
		mIp.resize(2 + int(std::sqrt(double(size))));
		mW.resize(size / 2);
		mBuffer.resize(size);
		mSize = size;

		const int size4 = size / 4;
		ooura::makewt(size4, mIp.data(), mW[0]);
		ooura::makect(size4, mIp.data(), mW[0] + size4);
	}

	template <typename T>
	void forward(const AudioBufferTpl<T>& input, AudioBufferTpl<T>& result) {
		T* real = result.get(0);
		T* imaginary = result.get(1);
		/**
		 * converts or copies the data
		 * we shouldn't use the original buffer since
		 * mBuffer gets written toby the fft
		 */
		mBuffer.set(input);
		ooura::rdft(mSize, +1, mBuffer[0], mIp.data(), mW.get(0));

		// Convert back to split-complex
		{
			double* b = mBuffer[0];
			double* bEnd = b + mSize;
			T* r = real;
			T* i = imaginary;
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
	void back(const AudioBufferTpl<T>& input, AudioBufferTpl<T>& result) {
		T* data = result.get(0);
		const T* real = input.get(0);
		const T* imaginary = input.get(1);
		{
			double* b = mBuffer.get(0);
			double* bEnd = b + mSize;
			const T* r = real;
			const T* i = imaginary;
			while (b != bEnd) {
				*(b++) = (*(r++));
				*(b++) = -(*(i++));
			}
			mBuffer[0][1] = real[mSize / 2];
		}
		ooura::rdft(int(mSize), -1, mBuffer.get(0), mIp.data(), mW.get(0));

		const T volume = 2.0 / T(mSize);
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

#endif // TKLB_FFT_OOURA
