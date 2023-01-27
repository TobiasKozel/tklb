/**
 * @file TWaveFile.hpp
 * @author Tobias Kozel
 * @brief Wrapper around dr_wav
 * @version 0.1
 * @date 2022-08-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef _TKLB_WAVE
#define _TKLB_WAVE

#include "./TAudioBuffer.hpp"

#define DR_WAV_NO_STDIO

#include "../../../external/dr_wav.h"

#ifdef TKLB_IMPL
	#include "./TWave.cpp"
#endif


namespace tklb {
	namespace wave {
		// TODO move this in a class or something
		namespace _ {
			void* drwaveMalloc(SizeT size, void* userData);
			void drwaveFree(void* ptr, void* userData);
		}

		/**
		 * @brief Decode wav from memory
		 * @param path The path or the wav file buffer if length is non 0
		 * @param out The buffer to store the result in
		 * @param length The length of the wav file buffer if not reading from file
		 */
		template <typename T, class Buffer = AudioBufferTpl<T>>
		bool load(const char* data, typename Buffer::Size length, Buffer& out) {
			drwav_allocation_callbacks drwaveCallbacks {
				nullptr,		// No userdata
				&_::drwaveMalloc,
				nullptr,		// TODO tklb test and add the realloc
				&_::drwaveFree
			};

			using Size = typename Buffer::Size;
			using uchar = unsigned char;

			drwav wav;

			if (!drwav_init_memory(
				&wav, data, length,
				&drwaveCallbacks
			)) {
				return false;
			}
			constexpr Size chunkSize = 128;
			Size read = 0;
			HeapBuffer<float> chunkBuffer;
			chunkBuffer.resize(chunkSize * wav.channels);

			out.sampleRate = wav.sampleRate;
			out.resize(Size(wav.totalPCMFrameCount), Size(wav.channels));

			while (read < wav.totalPCMFrameCount) {
				auto got = (Size) drwav_read_pcm_frames_f32(
					&wav, chunkSize, chunkBuffer.data()
				);
				out.setFromInterleaved(chunkBuffer.data(), got, uchar(wav.channels), 0, read);
				read += got;
			}

			if (read == 0) {
				return false;
			}

			out.setValidSize(read);
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
		 * @param in The audio buffer to write, needs samplerate to be set
		 * @param out Buffer to write the wave file to
		 * @param option Wave options to pass
		 */
		template <typename T, class Buffer = AudioBufferTpl<T>>
		bool write(
			const Buffer& in,
			HeapBuffer<char>& out,
			const WaveOptions&& options = {}
		) {
			drwav_allocation_callbacks drwaveCallbacks {
				nullptr,		// No userdata
				&_::drwaveMalloc,
				nullptr,		// TODO tklb test and add the realloc
				&_::drwaveFree
			};
			drwav wav;

			// TODO tklb this causes issues somehow
			// using Size = typename Buffer::Size;
			using Size = SizeT;

			drwav_data_format droptions;
			TKLB_ASSERT(in.sampleRate != 0) // Set a samplerate
			droptions.sampleRate = in.sampleRate;
			droptions.channels = in.channels();
			droptions.bitsPerSample = options.bitsPerSample;
			droptions.format = options.format;
			droptions.container = drwav_container(options.container);

			SizeT outSize = 0;
			void* memory = nullptr;

			if (!drwav_init_memory_write(
				&wav, &memory, &outSize,
				&droptions, &drwaveCallbacks)
			) {
				return false;
			}

			Size written = 0;
			constexpr Size chunkSize = 128;
			const Size frames = in.validSize();

			switch (options.format) {
			case WaveOptions::Format::IEEE_FLOAT:
				{
					// TODO benchmark against aligned heapbuffer
					auto interleaved = new float[chunkSize * in.channels()];
					while (written < frames) {
						auto remaining = in.putInterleaved(interleaved, chunkSize, written);
						written += (Size) drwav_write_pcm_frames(
							&wav, remaining, interleaved
						);
					}
					delete[] interleaved;
				}
				break;
			case WaveOptions::Format::PCM:
				{
					// TODO benchmark against aligned heapbuffer
					short* interleaved = (short*) TKLB_MALLOC(
						chunkSize * in.channels() * sizeof(short)
					);
					while (written < frames) {
						auto read = in.putInterleaved(interleaved, chunkSize, written);
						written += Size(drwav_write_pcm_frames(
							&wav, read, interleaved)
						);
					}
					TKLB_FREE(interleaved);
				}
				break;
			default:
				TKLB_ASSERT(false) return false;
			}

			out.set(reinterpret_cast<char*>(memory), Size(outSize));
			drwav_uninit(&wav);
			tklb_free(memory);
			return true;
		}
	}
} // namespace

#endif // _TKLB_WAVE
