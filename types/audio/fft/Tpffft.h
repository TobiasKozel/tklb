#ifndef TKLB_FFT_PFFFT
#define TKLB_FFT_PFFFT


#ifdef TKLB_NO_SIMD
	#define PFFFT_SIMD_DISABLE
#endif

#define PFFFT_NO_ALIGNMENT_CHECK
#include "../../../external/pffft/pffft.h"
#include "../../../external/pffft/pffft.c"

#include <cmath>
#include <vector>
#include "../TAudioBuffer.h"

namespace tklb {

class FFTpffft {
	using uchar = unsigned char;
	using uint = unsigned int;

	uint mSize;
	PFFFT_Setup* mSetup = nullptr;
	AudioBufferFloat mBuffer; //  Audiobuffer
	AudioBufferFloat mRc; // RealComplexBuffer
public:

	FFTpffft(uint size = 0) {
		if (size == 0) { return; }
		resize(size);
	}

	~FFTpffft() {
		if (mSetup == nullptr) { return; }
		pffft_destroy_setup(mSetup);
	}

	void resize(uint size) {
		if (mSetup != nullptr) {
			pffft_destroy_setup(mSetup);
		}
		mSetup = pffft_new_setup(size, PFFFT_REAL);
		mBuffer.resize(size);
		mRc.resize(size * 2);
		mSize = size;
	}

	template <typename T>
	void forward(const AudioBufferTpl<T>& input, AudioBufferTpl<T>& result) {
		const float* data = nullptr;
		if (std::is_same<T, float>::value) {
			data = reinterpret_cast<const float*>(input[0]);
		} else {
			mBuffer.set(input); // Type conversion
			data = mBuffer[0];
		}
		pffft_transform(mSetup, data, mRc[0], nullptr, PFFFT_FORWARD);
		// Split real and complex in two channels
		const uint sizeHalf = mSize / 2;
		result.set(mRc[0],            sizeHalf, 0);
		result.set(mRc[0] + sizeHalf, sizeHalf, 1);
	}

	template <typename T>
	void back(const AudioBufferTpl<T>& input, AudioBufferTpl<T>& result) {
		const uint sizeHalf = mSize / 2;
		mRc.set(input[0], sizeHalf);
		mRc.set(input[1], sizeHalf, 0, sizeHalf);
		const T volume = 1.0 / double(mSize);
		if (std::is_same<T, float>::value) {
			float* out = reinterpret_cast<float*>(result[0]);
			pffft_transform(mSetup, mRc[0], out, nullptr, PFFFT_BACKWARD);
			result.multiply(volume); // scale the result
		} else {
			pffft_transform(mSetup, mRc[0], mBuffer[0], nullptr, PFFFT_BACKWARD);
			const float* buf = mBuffer[0];
			T* out = result[0];
			for (uint i = 0; i < mSize; i++) {
				out[i] = buf[i] * volume; // scale the result + type conversion
			}
		}
	}
};

} // namespace

#endif // TKLB_FFT_PFFFT

