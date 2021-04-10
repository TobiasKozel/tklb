#ifndef TKLBZ_CONVOLVER_BRUTE
#define TKLBZ_CONVOLVER_BRUTE

#include <cmath>
#include "../../../util/TAssert.h"
#include "../../../util/TMath.hpp"
#include "./../TAudioBuffer.hpp"

namespace tklb {
	/**
	 * @brief Brute force convolver using element wise multiplication.
	 * Super slow especially for longer impulse responses
	 */
	template <typename T>
	class ConvolverBruteTpl {
		using uchar = unsigned char;
		using uint = unsigned int;
		using Buffer = AudioBufferTpl<T>;
		Buffer mIr; // Buffer holding the IR
	public:

		ConvolverBruteTpl() = default;

		/**
		 * @brief Load a impulse response and prepare the convolution
		 * @param buffer The ir buffer.
		 * @param channel Which channel to use from the AudioBuffer
		 * @param blockSize Size of blocks ir will be divided in
		 */
		template <typename T2>
		void load(const AudioBufferTpl<T2>& ir, const uint blockSize) {
			// trim silence, since longer IRs increase CPU usage considerably
			uint irLength = ir.size();
			if (irLength == 0) { return; }
			const T2 silence = 0.0001;

			for (uint i = irLength - 1; 0 < i; i--) {
				for (uchar c = 0; c < ir.channels(); c++) {
					if (silence < fabs(ir[c][i])) {
						// if any of the channels are over the
						// silence threshold we use the current
						// size and bail
						irLength = i + 1;
						goto endLoop;
					}
				}
			}
		endLoop:

			mIr.resize(irLength, ir.channels());
			mIr.set(ir);
		}

		/**
		 * @brief Do the convolution
		 * @param in Input signal, can be mono
		 * @param out Output buffer, needs to have enough space allocated
		 */
		template <typename T2>
		void process(const AudioBufferTpl<T2>& in, AudioBufferTpl<T2>& out) {
			const uint nf = mIr.size();
			const uint ng = in.validSize();
			const uint n = out.size();

			for (uchar c = 0; c < out.channels(); c++) {
				// eg the input is mono, but the IR stereo
				// the result will still be stereo
				const uchar inChannel = c % in.channels();

				// eg the ir is mono, it'll be used
				// for all channels
				const uchar irChannel = c % mIr.channels();

				for(uint i = 0; i < n; i++) {
					const uint jmn = (i >= ng - 1) ? (i - (ng - 1)) : 0;
					const uint jmx = (i <  nf - 1) ?  i : (nf - 1);
					for(uint j = jmn; j <= jmx; j++) {
						// nested loop goes brr
						out[c][i] += (mIr[irChannel][j] * in[inChannel][i - j]);
					}
				}
			}
		}
	};

	using ConvolverBruteFloat = ConvolverBruteTpl<float>;
	using ConvolverBruteDouble = ConvolverBruteTpl<double>;

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using ConvolverBrute = ConvolverBruteTpl<float>;
	#else
		using ConvolverBrute = ConvolverBruteTpl<double>;
	#endif

} // namespace

#endif // TKLB_CONVOLVER_BRUTE
