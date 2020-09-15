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
	AudioBufferFloat mBuffer = { 1 }; //  Audiobuffer
	AudioBufferFloat mRc = { 1 }; // RealComplexBuffer
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
			data = reinterpret_cast<const float*>(input.get(0));
		} else {
			mBuffer.set(input);
			data = mBuffer.get(0);
		}
		pffft_transform(mSetup, data, mRc.get(0), nullptr, PFFFT_FORWARD);
		const uint sizeHalf = mSize / 2;
		result.set(mRc.get(0), 0, sizeHalf);
		result.set(mRc.get(0) + sizeHalf, 1, sizeHalf);
	}

	template <typename T>
	void back(const AudioBufferTpl<T>& input, AudioBufferTpl<T>& result) {
		const uint sizeHalf = mSize / 2;
		mRc.set(input.get(0), 0, sizeHalf);
		mRc.set(input.get(1), 0, sizeHalf, sizeHalf);
		const T volume = 1.0 / static_cast<double>(mSize);
		if (std::is_same<T, float>::value) {
			float* out = reinterpret_cast<float*>(result.get(0));
			pffft_transform(mSetup, mRc.get(0), out, nullptr, PFFFT_BACKWARD);
			result.multiply(volume);
		} else {
			pffft_transform(mSetup, mRc.get(0), mBuffer.get(0), nullptr, PFFFT_BACKWARD);
			const float* buf = mBuffer.get(0);
			T* out = result.get(0);
			for (uint i = 0; i < mSize; i++) {
				out[i] = buf[i] * volume;
			}
		}
	}
};

} // namespace

#endif // TKLB_FFT_PFFFT

