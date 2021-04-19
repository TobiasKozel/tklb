#ifndef TKLBZ_AUDIOFILE
#define TKLBZ_AUDIOFILE

#include "../../memory/TMemory.hpp"
#include "./TAudioBuffer.hpp"

#ifdef TKLBZ_AUDIOFILE_IMPL
	#define DR_WAV_IMPLEMENTATION
#endif

#include "../../../external/dr_wav.h"

namespace tklb {
	namespace wave {

		namespace _ {
			void* drwaveMalloc(size_t size, void* userData) {
				return TKLB_MALLOC(size);
			}

			void drwaveFree(void* ptr, void* userData) {
				TKLB_FREE(ptr);
			}

			drwav_allocation_callbacks drwaveCallbacks {
				nullptr,		// No userdata
				drwaveMalloc,
				nullptr,		// TODO tklb test and add the realloc
				drwaveFree
			};
		}

		/**
		 * @brief Decode wav from memory or file path
		 * @param path The path or the wav file buffer if length is 0
		 * @param out The buffer to store the result in
		 * @param length The length of the wav file buffer if not reading from file
		 */
		template <typename T>
		bool load(const char* path, AudioBufferTpl<T>& out, typename AudioBufferTpl<T>::Size length = 0) {
			drwav wav;
			if (length == 0) {
				if (!drwav_init_file(&wav, path, &_::drwaveCallbacks)) {
					return false;
				}
			} else {
				if (!drwav_init_memory(&wav, path, length, &_::drwaveCallbacks)) {
					return false;
				}
			}

			float* sampleData = TKLB_MALLOC(
				size_t(wav.totalPCMFrameCount) * size_t(wav.channels) * sizeof(float)
			);

			if (sampleData == nullptr) { return false; }

			length = drwav_read_pcm_frames_f32(
				&wav, wav.totalPCMFrameCount, sampleData
			);

			if (length == 0) {
				TKLB_FREE(sampleData);
				return false;
			}
			out.sampleRate = wav.sampleRate;
			out.resize(length, wav.channels);
			out.setFromInterleaved(sampleData, length, wav.channels);
			out.setValidSize(length);
			TKLB_FREE(sampleData);
			drwav_uninit(&wav);
		}

		template <typename T>
		bool write(const AudioBufferTpl<T>& in, char* path, typename AudioBufferTpl<T> length = 0) {
			// TODO tklb
			TKLB_ASSERT(false);
		}

	}

} // namespace

#endif // TKLB_AUDIOFILE
