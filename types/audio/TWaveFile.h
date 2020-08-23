#ifndef TKLB_AUDIOFILE
#define TKLB_AUDIOFILE

#define DR_WAV_IMPLEMENTATION
#include "../../external/dr_wav.h"

namespace tklb {

class WaveFile {
	bool mValid = false;
	float** mSamples = nullptr;
	unsigned int mChannels = 0;
	unsigned int mSampleRate = 0;
	unsigned int mLength = 0;
public:
	/**
	 * Load wave from file
	 */
	WaveFile(const char* file = nullptr) {
		if (file == nullptr) { return; }
		drwav wav;
		if (!drwav_init_file(&wav, file, nullptr)) {
			return;
		}

	}


	/**
	 * Load wave from memory
	 */
	WaveFile(const char* data, unsigned int length) {
		drwav wav;
		if (!drwav_init_memory(&wav, data, length, nullptr)) {
			return;
		}
	}

	bool getValid() const {
		return mValid;
	}

private:
	bool decode(drwav* wav) {
		float* sampleData = static_cast<float*>(malloc(
			static_cast<size_t>(wav->totalPCMFrameCount)
			* wav->channels * sizeof(float)
		));

		if (sampleData == nullptr) { return false; }
		size_t length = drwav_read_pcm_frames_f32(
			wav, wav->totalPCMFrameCount, sampleData
		);
		if (length == 0) {
			free(sampleData);
			return false;
		}
		int channels = wav->channels;

		mLength = length;
		mChannels = channels;
		mSampleRate = wav->sampleRate;
	}
};

} // namespace

#endif // TKLB_AUDIOFILE
