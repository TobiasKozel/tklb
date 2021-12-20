#ifndef _TKLB_OGG_FILE
#define _TKLB_OGG_FILE

#include "../../memory/TMemory.hpp"
#include "./TAudioBuffer.hpp"


#define STB_VORBIS_NO_PUSHDATA_API // Don't need this
#define STB_VORBIS_NO_INTEGER_CONVERSION // don't need either
#include "../../../external/stb_vorbis.c"
#include <type_traits>

namespace tklb {
	namespace ogg {

		/**
		 * @brief Decode ogg/vorbis from memory or file path
		 * @param path The path or the wav file buffer if length is non 0
		 * @param out The buffer to store the result in
		 * @param length The length of the wav file buffer if not reading from file
		 */
		template <typename T>
		bool load(const char* path, AudioBufferTpl<T>& out, typename AudioBufferTpl<T>::Size length = 0) {
			using Size = typename AudioBufferTpl<T>::Size;
			using uchar = typename AudioBufferTpl<T>::uchar;
			using ushort = typename AudioBufferTpl<T>::ushort;

			stb_vorbis* vorbis;
			int error = 0;
			if (length == 0) {
				vorbis = stb_vorbis_open_filename(path, &error, nullptr);
			} else {
				vorbis = stb_vorbis_open_memory(
					reinterpret_cast<const unsigned char*>(path), length, &error, nullptr
				);
			}

			if (error != VORBIS__no_error) {
				stb_vorbis_close(vorbis);
				return false;
			}

			const stb_vorbis_info info = stb_vorbis_get_info(vorbis);
			const Size streamLength = stb_vorbis_stream_length_in_samples(vorbis);


			out.sampleRate = info.sample_rate;
			out.resize(streamLength, Size(info.channels)); // will fail at channels higher than allowed

			if (std::is_same<float, T>::value) {
				float* buffer[out.maxChannels()];
				out.getRaw(buffer);
				Size got = stb_vorbis_get_samples_float(vorbis, info.channels, buffer, streamLength);
				out.setValidSize(got);
				stb_vorbis_close(vorbis);
				return got == streamLength;
			}

			Size read = 0;

			int channels = info.channels;
			float** bufferPtr = nullptr;
			while (read < streamLength) {
				Size got = stb_vorbis_get_frame_float(vorbis, &channels, &bufferPtr);
				out.set(bufferPtr, got, channels, 0, read);
				read += got;
			}

			out.setValidSize(read);

			stb_vorbis_close(vorbis);

			if (read == 0) {
				return false;
			}

			return true;
		}
	}
} // namespace

#endif // _TKLB_OGG_FILE
