#ifndef _TKLB_ICODEC
#define _TKLB_ICODEC

#include "../../THeapBuffer.hpp"
#include "../TAudioBuffer.hpp"

namespace tklb {
	template <typename T, class Buffer = AudioBufferTpl<T>>
	struct ICodecTpl {
		using Size = HeapBuffer<>::Size;
		enum class Result {
			Success = 0,
			GenericError,
			NotImplemented
			// TODO
		};

		/**
		 * @brief TODO
		 *
		 * @param path
		 * @return Result
		 */
		virtual Result open(const void* file) = 0;

		/**
		 * @brief TODO
		 *
		 * @return Result
		 */
		virtual Result readAll() = 0;

		/**
		 * @brief TODO
		 *
		 * @param count
		 * @return Size
		 */
		virtual Size read(Size count, Buffer& result) = 0;

		/**
		 * @brief TODO
		 *
		 * @param audio
		 * @return Size
		 */
		virtual Result write(const Buffer& audio) = 0;

		/**
		 * @brief TODO
		 *
		 * @param index
		 * @return Result
		 */
		virtual Result scrub(Size index) = 0;
	};
} // tklb

#endif // _TKLB_ICODEC
