#ifndef TKLBZ_WAVE_FILE
#define TKLBZ_WAVE_FILE

#include "../../memory/TMemory.hpp"
#include "./TAudioBuffer.hpp"

#include "../../../external/dr_wav.h"

#ifdef TKLBZ_WAVE_FILE_IMPL
	#include "./TWaveFile.cpp"
#endif

namespace tklb {
	namespace wave {
		/**
		 * redirects malloc for drwav, not really needed since
		 * loading and saving wavs doesn't seem to allocate
		 * anything on the heap
		 */
		namespace _ {
			void* drwaveMalloc(size_t size, void* userData) {
				return TKLB_MALLOC(size);
			}

			void drwaveFree(void* ptr, void* userData) {
				TKLB_FREE(ptr);
			}

			drwav_allocation_callbacks drwaveCallbacks {
				nullptr,		// No userdata
				&drwaveMalloc,
				nullptr,		// TODO tklb test and add the realloc
				&drwaveFree
			};
		}

		/**
		 * @brief Decode wav from memory or file path
		 * @param path The path or the wav file buffer if length is non 0
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

			float* sampleData =
				reinterpret_cast<float*>(
					TKLB_MALLOC_ALIGNED(size_t(wav.totalPCMFrameCount) * size_t(wav.channels) * sizeof(float))
				);

			if (sampleData == nullptr) { return false; }

			length = drwav_read_pcm_frames_f32(
				&wav, wav.totalPCMFrameCount, sampleData
			);

			if (length == 0) {
				TKLB_FREE_ALIGNED(sampleData);
				return false;
			}
			out.sampleRate = wav.sampleRate;
			out.resize(length, wav.channels);
			out.setFromInterleaved(sampleData, length, wav.channels);
			out.setValidSize(length);
			TKLB_FREE_ALIGNED(sampleData);
			drwav_uninit(&wav);
			return true;
		}

		/**
		 * @brief Small option struct to save wave files
		 */
		struct WaveOptions {
			int bitsPerSample = 32;
			enum Container {
				riff = drwav_container_riff,
				w64 = drwav_container_w64,
				rf64 = drwav_container_rf64
			};
			Container container = Container::riff;

			enum Format {
				PCM = DR_WAVE_FORMAT_PCM,
				// ADPCM = DR_WAVE_FORMAT_ADPCM,
				IEEE_FLOAT = DR_WAVE_FORMAT_IEEE_FLOAT,
				// ALAW = DR_WAVE_FORMAT_ALAW,
				// MULAW = DR_WAVE_FORMAT_MULAW,
				// DVI_ADPCM = DR_WAVE_FORMAT_DVI_ADPCM,
				// EXTENSIBLE = DR_WAVE_FORMAT_EXTENSIBLE
			};
			Format format = Format::IEEE_FLOAT;
		};

		/**
		 * @brief Write audiobuffer to file or memory
		 */
		template <typename T>
		bool write(
			const AudioBufferTpl<T>& in,
			const char* path,
			const WaveOptions&& options = {},
			HeapBuffer<char>* out = nullptr
		) {
			drwav wav;

			drwav_data_format droptions;
			droptions.sampleRate = in.sampleRate;
			droptions.channels = in.channels();
			droptions.bitsPerSample = options.bitsPerSample;
			droptions.format = options.format;
			droptions.container = drwav_container(options.container);

			size_t outSize = 0;
			void* memory = nullptr;
			if (out == nullptr) { // write to file
				if (!drwav_init_file_write(&wav, path, &droptions, &_::drwaveCallbacks)) {
					return false;
				}
			} else {
				if (!drwav_init_memory_write(&wav, &memory, &outSize, &droptions, &_::drwaveCallbacks)) {
					return false;
				}
			}

			size_t written = 0;
			using Size = HeapBuffer<float>::Size;
			const Size chunkSize = 128;
			const size_t frames = in.validSize();

			switch (options.format) {
			case WaveOptions::Format::IEEE_FLOAT:
				{
					// TODO benchmark against aligned heapbuffer
					float interleaved[chunkSize * in.channels()];
					while (written < frames) {
						auto remaining = in.putInterleaved(interleaved, chunkSize, written);
						remaining /= in.channels();
						written += drwav_write_pcm_frames(&wav, remaining, interleaved);
					}
				}
				break;
			case WaveOptions::Format::PCM:
				{
					// TODO benchmark against aligned heapbuffer
					short interleaved[chunkSize * in.channels()];
					while (written < frames) {
						auto read = in.putInterleaved(interleaved, chunkSize, written);
						written += drwav_write_pcm_frames(&wav, read, interleaved);
					}
				}
				break;
			default:
				TKLB_ASSERT(false) return false;
			}

			drwav_uninit(&wav);

			if (out != nullptr) {
				out->set(reinterpret_cast<char*>(memory), outSize);
			}

			return true;
		}
	}
} // namespace

#endif // TKLBZ_WAVE_FILE
