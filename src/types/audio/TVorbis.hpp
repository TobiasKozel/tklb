#ifndef _TKLB_VORBIS
#define _TKLB_VORBIS

#include "./TAudioBuffer.hpp"


#define STB_VORBIS_NO_PUSHDATA_API // Don't need this
#define STB_VORBIS_NO_INTEGER_CONVERSION // don't need either
// #define STB_VORBIS_NO_CRT // TODO hook the allocations
#define STB_VORBIS_NO_STDIO
#ifndef TKLB_IMPL
	// Works inverted
	#define STB_VORBIS_HEADER_ONLY
#endif
#include "../../../external/stb_vorbis.c"

namespace tklb { namespace vorbis {
	/**
	 * @brief Decode ogg/vorbis from memory or
	 * @param data The path or the wav file buffer if length is non 0
	 * @param length The length of the wav file buffer if not reading from file
	 * @param out The buffer to store the result in
	 */
	template <typename T, class Buffer = AudioBufferTpl<T>>
	bool load(const char* data, typename Buffer::Size length, Buffer& out) {
		using Size = typename Buffer::Size;

		stb_vorbis* vorbis;
		int error = 0;
		vorbis = stb_vorbis_open_memory(
			reinterpret_cast<const unsigned char*>(data), length, &error, nullptr
		);

		if (error != VORBIS__no_error) {
			stb_vorbis_close(vorbis);
			return false;
		}

		const stb_vorbis_info info = stb_vorbis_get_info(vorbis);
		const Size streamLength = stb_vorbis_stream_length_in_samples(vorbis);

		out.sampleRate = info.sample_rate;
		out.resize(streamLength, Size(info.channels)); // will fail at channels higher than allowed

		if (std::is_same<float, T>::value) {
			HeapBuffer<T*> buffer;
			buffer.resize(info.channels);
			out.getRaw(buffer.data());
			// seems evil but where only here if it's alredy a float**
			float** fbuf = reinterpret_cast<float**>(buffer.data());
			Size got = stb_vorbis_get_samples_float(
				vorbis, info.channels, fbuf, streamLength
			);
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
} } // tklb::vorbis

#endif // _TKLB_VORBIS
