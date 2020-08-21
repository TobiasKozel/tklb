#ifndef TKLB_RESAMPLER
#define TKLB_RESAMPLER

#include "../../external/SpeexResampler.h"
namespace tklb {

template <unsigned int CHANNELS = 1, unsigned int MAX_BLOCK = 512>
class Resampler {
	speexport::SpeexResampler mResampler;
	float mBuffer[CHANNELS][MAX_BLOCK] = { 0 };
public:
	Resampler(int rateIn, int rateOut, int quality = 5) {
		init(rateIn, rateOut, quality);
	}

	bool init(int rateIn, int rateOut, int quality = 5) {
		int err;
		mResampler.init(CHANNELS, rateIn, rateOut, quality, &err);
		return err == 0;
	}

	unsigned int process(float** buffer, unsigned int countIn) {
		unsigned int countOut = MAX_BLOCK;
		for (unsigned int c = 0; c < CHANNELS; c++) {
			unsigned int _countOut = MAX_BLOCK;
			unsigned int _countIn = countIn;
			mResampler.process(c, buffer[c], &_countIn, mBuffer[c], &_countOut);
			countOut = _countOut > countOut ? countOut : _countOut;
		}
		return countOut;
	}

	const float** getOutBuffer() const{
		return mBuffer;
	}
};

template <int MAX_BLOCK = 512, int MAX_CHANNELS = 16>
class HeapResampler {
	using ResamplerSingle = Resampler<1, MAX_BLOCK>;
	ResamplerSingle* mResamplers[MAX_CHANNELS] = { nullptr };
	unsigned int mChannels = 0;
public:
	HeapResampler(int rateIn, int rateOut, int channels = 2, int quality = 5) {
		init(rateIn, rateOut, channels, quality);
	}

	~HeapResampler() {
		for (int c = 0; c < MAX_CHANNELS; c++) {
			delete mResamplers[c];
		}
	}

	bool init(int rateIn, int rateOut, int channels = 2, int quality = 5) {
		mChannels = channels;
		for (int c = 0; c < MAX_CHANNELS; c++) {
			delete mResamplers[c];
			mResamplers[c] = nullptr;
		}

		for (int c = 0; c < channels; c++) {
			mResamplers[c] = new ResamplerSingle(rateIn, rateOut, quality);
		}
		return true;
	}

	unsigned int process(float** buffer, unsigned int countIn) {
		unsigned int countOut = MAX_BLOCK;
		for (unsigned int c = 0; c < mChannels; c++) {
			float* channel[] = { buffer[c] };
			unsigned int _countOut = mResamplers[c]->process(channel, countIn);
			countOut = _countOut > countOut ? countOut : _countOut;
		}
		return countOut;
	}
};

} // namespace

#endif // RESAMPLER
