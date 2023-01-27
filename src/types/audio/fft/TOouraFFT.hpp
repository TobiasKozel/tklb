#ifndef _TKLB_FFT_OOURA
#define _TKLB_FFT_OOURA

#include "../TAudioBuffer.hpp"
#include "../../../util/TTraits.hpp"
#include "../../../util/TMath.hpp"
#include <cstdio>

namespace tklb {
	/**
	 * @brief Wrapper around the Ooura
	 * @tparam T Type used by ooura, float might be wildly inaccurate or broken.
	 * TODO Expose storage
	 */
	template <typename T = double>
	class FFTOouraTpl {
		using Size = typename AudioBufferTpl<T>::Size;

		HeapBuffer<int, 16> mIp;	///< No idea what this is
		HeapBuffer<T, 16> mW;		///< or this, prolly lookup tables
		AudioBufferTpl<T> mBuffer;	///< Working buffer

	public:
		FFTOouraTpl(Size size) {
			TKLB_ASSERT(size != 0 && isPowerof2(size))
			// Pretty HiFi-LoFi AudioFFT
			mIp.resize(2 + Size(tklb::sqrt(T(size))));
			mW.resize(size / 2);
			mBuffer.resize(size);

			const auto size4 = size / 4;
			makewt(size4, mIp.data(), mW.data());
			makect(size4, mIp.data(), mW.data() + size4);
		}

		/**
		 * @brief Gets the space the fft result will need
		 */
		Size resultSize(const Size inputLength) const {
			const auto size = mBuffer.size();
			const auto sizeHalf = size / 2;
			const auto fftResultBlockSize = sizeHalf + 1;
			const auto blocks = inputLength / size;
			return fftResultBlockSize * blocks;
		}


		/**
		 * @brief timedomain to frequency domain
		 * @param input Input buffer time domain, validSize needs
		                to be devisible by the fft size.
		 * @param output Output buffer, Frequency Domain.
		 *               Must have 2 channel for real and imaginary and
		 *               half the total length of the input buffer + 1.
		 */
		template <typename T2>
		void forward(const AudioBufferTpl<T2>& input, AudioBufferTpl<T2>& output) {
			const auto size = mBuffer.size();
			const auto sizeHalf = size / 2;
			const auto fftResultBlockSize = sizeHalf + 1;
			const auto blocks = input.validSize() / size;

			TKLB_ASSERT(output.channels() == 2)
			TKLB_ASSERT((input.validSize() % size) == 0)
			TKLB_ASSERT((fftResultBlockSize * blocks) <= output.size())

			const T2* in = input[0];
			const T2* inEnd = in + input.validSize();
			T2* real = output[0];
			T2* imaginary = output[1];

			while (in != inEnd) {
				mBuffer.set(in, mBuffer.size());
				rdft(mBuffer.size(), +1, mBuffer[0], mIp.data(), mW.data());

				// Convert back to split-complex
				{
					// deinterleave the ooura output
					auto b = mBuffer[0];
					auto bEnd = b + mBuffer.size();
					auto r = real;
					auto i = imaginary;
					while (b != bEnd) {
						*(r++) = (*(b++));
						// the sign of the imaginary part is flipped
						*(i++) = (-(*(b++)));
					}

					// not sure if i'm understanding this whole thing right
					const auto sizeHalf = mBuffer.size() / 2;
					// put dc offset at the end of the real part
					real[sizeHalf] = -imaginary[0];
					// clear out the offset from the complex part
					imaginary[0] = 0.0;
					// zero the excess complex part at the end for safety
					imaginary[sizeHalf] = 0.0;
				}

				in += size;
				real += fftResultBlockSize;
				imaginary += fftResultBlockSize;
			}

			output.setValidSize(fftResultBlockSize * blocks);
		}

		/**
		 * @brief Frequency domain back to time domain
		 * @param input Frequency Domain Buffer with 2 channels.
		 *        channel 0 for real and 1 for imaginary
		 * @param result Single channel output buffer.
		 *        Needs to be twice the size of the imput buffer
		 */
		template <typename T2>
		void back(const AudioBufferTpl<T2>& input, AudioBufferTpl<T2>& output) {
			const auto size = mBuffer.size();
			const auto sizeHalf = size / 2;
			const auto fftResultBlockSize = sizeHalf + 1;
			const auto blocks = input.validSize() / fftResultBlockSize;

			TKLB_ASSERT(input.channels() == 2)
			TKLB_ASSERT((input.validSize() % fftResultBlockSize) == 0)
			TKLB_ASSERT((blocks * size) <= output.size())

			T2* out = output[0];
			const T2* real = input[0];
			const T2* realEnd = real + input.validSize();
			const T2* imaginary = input[1];

			while (real != realEnd) {
				{
					auto b = mBuffer[0];
					auto bEnd = b + size;
					const T* r = real;
					const T* i = imaginary;
					while (b != bEnd) {
						*(b++) = (*(r++));
						*(b++) = -(*(i++));
					}
					mBuffer[0][1] = real[sizeHalf];
				}

				rdft(size, -1, mBuffer[0], mIp.data(), mW.data());

				const T2 volume = 2.0 / T2(mBuffer.size());
				if (traits::IsSame<T2, T>::value) {
					mBuffer.multiply(volume); // scale the output
					mBuffer.put(reinterpret_cast<T*>(out), mBuffer.size());
				} else {
					// or the old fashioned way if we need to convert sample types anyways
					const T* buf = mBuffer[0];
					for (Size i = 0; i < mBuffer.size(); i++) {
						out[i] = buf[i] * volume;
					}
				}
				out += size;
				real += fftResultBlockSize;
				imaginary += fftResultBlockSize;
			}
		}
	private:

		/**
		* Slightly altered OOURA FFT below.
		* Implementation from (url seems to have changed over the years):
		* http://www.kurims.kyoto-u.ac.jp/~ooura/fft.html
		*
		* Original License below:
		*
        * Copyright(C) 1996-2001 Takuya OOURA
        * email: ooura@mmm.t.u-tokyo.ac.jp
        * download: http://momonga.t.u-tokyo.ac.jp/~ooura/fft.html
        * You may use, copy, modify this code for any purpose and
        * without fee. You may distribute this ORIGINAL package.
		*/

		/**
		 * @brief Real Discrete Fourier Transform
		 *        A method with a following butterfly operation appended to "cdft"
		 *        (Complex Discrete Fourier Transform).
		 *        In forward transform :
		 *            A[k] = sum_j=0^n-1 a[j]*W(n)^(j*k), 0<=k<=n/2,
		 *                W(n) = exp(2*pi*i/n),
		 *        this routine makes an array x[] :
		 *            x[j] = a[2*j] + i*a[2*j+1], 0<=j<n/2
		 *        and calls "cdft" of length n/2 :
		 *            X[k] = sum_j=0^n/2-1 x[j] * W(n/2)^(j*k), 0<=k<n.
		 *        The result A[k] are :
		 *            A[k]     = X[k]     - (1+i*W(n)^k)/2 * (X[k]-conjg(X[n/2-k])),
		 *            A[n/2-k] = X[n/2-k] +
		 *                            conjg((1+i*W(n)^k)/2 * (X[k]-conjg(X[n/2-k]))),
		 *                0<=k<=n/2
		 *            (notes: conjg() is a complex conjugate, X[n/2]=X[0]).
		 * @param n FFT Size
		 * @param isgn +1 for forward dft and -1 for backwards dft.
		 * @param a input/output buffer
		 * @param ip TODO better description
		 * @param w TODO better description
		 */
		static inline void rdft(int n, int isgn, T* a, int* ip, T* w) {
			int nw = ip[0];
			int nc = ip[1];
			if (isgn >= 0) {
				if (n > 4) {
					bitrv2(n, ip + 2, a);
					cftfsub(n, a, w);
					rftfsub(n, a, nc, w + nw);
				} else if (n == 4) {
					cftfsub(n, a, w);
				}
				T xi = a[0] - a[1];
				a[0] += a[1];
				a[1] = xi;
			} else {
				a[1] = 0.5 * (a[0] - a[1]);
				a[0] -= a[1];
				if (n > 4) {
					rftbsub(n, a, nc, w + nw);
					bitrv2(n, ip + 2, a);
					cftbsub(n, a, w);
				} else if (n == 4) {
					cftfsub(n, a, w);
				}
			}
		}

		/**
		 * @brief Initialization
		 *
		 * @param nw
		 * @param ip
		 * @param w
		 */
		static inline void makewt(int nw, int* ip, T* w) {
			int j, nwh;
			T delta, x, y;
			ip[0] = nw;
			ip[1] = 1;
			if (nw > 2) {
				nwh = nw >> 1;
				delta = tklb::atan(1.0) / nwh;
				w[0] = 1;
				w[1] = 0;
				w[nwh] = tklb::cos(delta * nwh);
				w[nwh + 1] = w[nwh];
				if (nwh > 2) {
					for (j = 2; j < nwh; j += 2) {
						x = tklb::cos(delta * j);
						y = tklb::sin(delta * j);
						w[j] = x;
						w[j + 1] = y;
						w[nw - j] = y;
						w[nw - j + 1] = x;
					}
					bitrv2(nw, ip + 2, w);
				}
			}
		}

		/**
		 * @brief Initialiation
		 *
		 * @param nc
		 * @param ip
		 * @param c
		 */
		static inline void makect(int nc, int* ip, T* c) {
			int j, nch;
			T delta;
			ip[1] = nc;
			if (nc > 1) {
				nch = nc >> 1;
				delta = tklb::atan(1.0) / nch;
				c[0] = tklb::cos(delta * nch);
				c[nch] = 0.5 * c[0];
				for (j = 1; j < nch; j++) {
					c[j] = 0.5 * tklb::cos(delta * j);
					c[nc - j] = 0.5 * tklb::sin(delta * j);
				}
			}
		}

		/**
		 * @brief Child routine
		 *
		 * @param n
		 * @param ip
		 * @param a
		 */
		static inline void bitrv2(int n, int* ip, T* a) {
			int j, j1, k, k1, l, m, m2;
			T xr, xi, yr, yi;
			ip[0] = 0;
			l = n;
			m = 1;
			while ((m << 3) < l) {
				l >>= 1;
				for (j = 0; j < m; j++) {
					ip[m + j] = ip[j] + l;
				}
				m <<= 1;
			}
			m2 = 2 * m;
			if ((m << 3) == l) {
				for (k = 0; k < m; k++) {
					for (j = 0; j < k; j++) {
						j1 = 2 * j + ip[k];
						k1 = 2 * k + ip[j];
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 += 2 * m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 -= m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 += 2 * m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
					}
					j1 = 2 * k + m2 + ip[k];
					k1 = j1 + m2;
					xr = a[j1];
					xi = a[j1 + 1];
					yr = a[k1];
					yi = a[k1 + 1];
					a[j1] = yr;
					a[j1 + 1] = yi;
					a[k1] = xr;
					a[k1 + 1] = xi;
				}
			} else {
				for (k = 1; k < m; k++) {
					for (j = 0; j < k; j++) {
						j1 = 2 * j + ip[k];
						k1 = 2 * k + ip[j];
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
						j1 += m2;
						k1 += m2;
						xr = a[j1];
						xi = a[j1 + 1];
						yr = a[k1];
						yi = a[k1 + 1];
						a[j1] = yr;
						a[j1 + 1] = yi;
						a[k1] = xr;
						a[k1 + 1] = xi;
					}
				}
			}
		}

		static inline void cftfsub(int n, T* a, T* w) {
			int j, j1, j2, j3, l;
			T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
			l = 2;
			if (n > 8) {
				cft1st(n, a, w);
				l = 8;
				while ((l << 2) < n) {
					cftmdl(n, l, a, w);
					l <<= 2;
				}
			}
			if ((l << 2) == n) {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = a[j + 1] + a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = a[j + 1] - a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i + x2i;
					a[j2] = x0r - x2r;
					a[j2 + 1] = x0i - x2i;
					a[j1] = x1r - x3i;
					a[j1 + 1] = x1i + x3r;
					a[j3] = x1r + x3i;
					a[j3 + 1] = x1i - x3r;
				}
			} else {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					x0r = a[j] - a[j1];
					x0i = a[j + 1] - a[j1 + 1];
					a[j] += a[j1];
					a[j + 1] += a[j1 + 1];
					a[j1] = x0r;
					a[j1 + 1] = x0i;
				}
			}
		}

		static inline void cftbsub(int n, T* a, T* w) {
			int j, j1, j2, j3, l;
			T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
			l = 2;
			if (n > 8) {
				cft1st(n, a, w);
				l = 8;
				while ((l << 2) < n) {
					cftmdl(n, l, a, w);
					l <<= 2;
				}
			}
			if ((l << 2) == n) {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = -a[j + 1] - a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = -a[j + 1] + a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i - x2i;
					a[j2] = x0r - x2r;
					a[j2 + 1] = x0i + x2i;
					a[j1] = x1r - x3i;
					a[j1 + 1] = x1i - x3r;
					a[j3] = x1r + x3i;
					a[j3 + 1] = x1i + x3r;
				}
			} else {
				for (j = 0; j < l; j += 2) {
					j1 = j + l;
					x0r = a[j] - a[j1];
					x0i = -a[j + 1] + a[j1 + 1];
					a[j] += a[j1];
					a[j + 1] = -a[j + 1] - a[j1 + 1];
					a[j1] = x0r;
					a[j1 + 1] = x0i;
				}
			}
		}

		static inline void cft1st(int n, T* a, T* w) {
			int j, k1, k2;
			T wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
			T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
			x0r = a[0] + a[2];
			x0i = a[1] + a[3];
			x1r = a[0] - a[2];
			x1i = a[1] - a[3];
			x2r = a[4] + a[6];
			x2i = a[5] + a[7];
			x3r = a[4] - a[6];
			x3i = a[5] - a[7];
			a[0] = x0r + x2r;
			a[1] = x0i + x2i;
			a[4] = x0r - x2r;
			a[5] = x0i - x2i;
			a[2] = x1r - x3i;
			a[3] = x1i + x3r;
			a[6] = x1r + x3i;
			a[7] = x1i - x3r;
			wk1r = w[2];
			x0r = a[8] + a[10];
			x0i = a[9] + a[11];
			x1r = a[8] - a[10];
			x1i = a[9] - a[11];
			x2r = a[12] + a[14];
			x2i = a[13] + a[15];
			x3r = a[12] - a[14];
			x3i = a[13] - a[15];
			a[8] = x0r + x2r;
			a[9] = x0i + x2i;
			a[12] = x2i - x0i;
			a[13] = x0r - x2r;
			x0r = x1r - x3i;
			x0i = x1i + x3r;
			a[10] = wk1r * (x0r - x0i);
			a[11] = wk1r * (x0r + x0i);
			x0r = x3i + x1r;
			x0i = x3r - x1i;
			a[14] = wk1r * (x0i - x0r);
			a[15] = wk1r * (x0i + x0r);
			k1 = 0;
			for (j = 16; j < n; j += 16) {
				k1 += 2;
				k2 = 2 * k1;
				wk2r = w[k1];
				wk2i = w[k1 + 1];
				wk1r = w[k2];
				wk1i = w[k2 + 1];
				wk3r = wk1r - 2 * wk2i * wk1i;
				wk3i = 2 * wk2i * wk1r - wk1i;
				x0r = a[j] + a[j + 2];
				x0i = a[j + 1] + a[j + 3];
				x1r = a[j] - a[j + 2];
				x1i = a[j + 1] - a[j + 3];
				x2r = a[j + 4] + a[j + 6];
				x2i = a[j + 5] + a[j + 7];
				x3r = a[j + 4] - a[j + 6];
				x3i = a[j + 5] - a[j + 7];
				a[j] = x0r + x2r;
				a[j + 1] = x0i + x2i;
				x0r -= x2r;
				x0i -= x2i;
				a[j + 4] = wk2r * x0r - wk2i * x0i;
				a[j + 5] = wk2r * x0i + wk2i * x0r;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j + 2] = wk1r * x0r - wk1i * x0i;
				a[j + 3] = wk1r * x0i + wk1i * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j + 6] = wk3r * x0r - wk3i * x0i;
				a[j + 7] = wk3r * x0i + wk3i * x0r;
				wk1r = w[k2 + 2];
				wk1i = w[k2 + 3];
				wk3r = wk1r - 2 * wk2r * wk1i;
				wk3i = 2 * wk2r * wk1r - wk1i;
				x0r = a[j + 8] + a[j + 10];
				x0i = a[j + 9] + a[j + 11];
				x1r = a[j + 8] - a[j + 10];
				x1i = a[j + 9] - a[j + 11];
				x2r = a[j + 12] + a[j + 14];
				x2i = a[j + 13] + a[j + 15];
				x3r = a[j + 12] - a[j + 14];
				x3i = a[j + 13] - a[j + 15];
				a[j + 8] = x0r + x2r;
				a[j + 9] = x0i + x2i;
				x0r -= x2r;
				x0i -= x2i;
				a[j + 12] = -wk2i * x0r - wk2r * x0i;
				a[j + 13] = -wk2i * x0i + wk2r * x0r;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j + 10] = wk1r * x0r - wk1i * x0i;
				a[j + 11] = wk1r * x0i + wk1i * x0r;
				x0r = x1r + x3i;
				x0i = x1i - x3r;
				a[j + 14] = wk3r * x0r - wk3i * x0i;
				a[j + 15] = wk3r * x0i + wk3i * x0r;
			}
		}

		static inline void cftmdl(int n, int l, T* a, T* w) {
			int j, j1, j2, j3, k, k1, k2, m, m2;
			T wk1r, wk1i, wk2r, wk2i, wk3r, wk3i;
			T x0r, x0i, x1r, x1i, x2r, x2i, x3r, x3i;
			m = l << 2;
			for (j = 0; j < l; j += 2) {
				j1 = j + l;
				j2 = j1 + l;
				j3 = j2 + l;
				x0r = a[j] + a[j1];
				x0i = a[j + 1] + a[j1 + 1];
				x1r = a[j] - a[j1];
				x1i = a[j + 1] - a[j1 + 1];
				x2r = a[j2] + a[j3];
				x2i = a[j2 + 1] + a[j3 + 1];
				x3r = a[j2] - a[j3];
				x3i = a[j2 + 1] - a[j3 + 1];
				a[j] = x0r + x2r;
				a[j + 1] = x0i + x2i;
				a[j2] = x0r - x2r;
				a[j2 + 1] = x0i - x2i;
				a[j1] = x1r - x3i;
				a[j1 + 1] = x1i + x3r;
				a[j3] = x1r + x3i;
				a[j3 + 1] = x1i - x3r;
			}
			wk1r = w[2];
			for (j = m; j < l + m; j += 2) {
				j1 = j + l;
				j2 = j1 + l;
				j3 = j2 + l;
				x0r = a[j] + a[j1];
				x0i = a[j + 1] + a[j1 + 1];
				x1r = a[j] - a[j1];
				x1i = a[j + 1] - a[j1 + 1];
				x2r = a[j2] + a[j3];
				x2i = a[j2 + 1] + a[j3 + 1];
				x3r = a[j2] - a[j3];
				x3i = a[j2 + 1] - a[j3 + 1];
				a[j] = x0r + x2r;
				a[j + 1] = x0i + x2i;
				a[j2] = x2i - x0i;
				a[j2 + 1] = x0r - x2r;
				x0r = x1r - x3i;
				x0i = x1i + x3r;
				a[j1] = wk1r * (x0r - x0i);
				a[j1 + 1] = wk1r * (x0r + x0i);
				x0r = x3i + x1r;
				x0i = x3r - x1i;
				a[j3] = wk1r * (x0i - x0r);
				a[j3 + 1] = wk1r * (x0i + x0r);
			}
			k1 = 0;
			m2 = 2 * m;
			for (k = m2; k < n; k += m2) {
				k1 += 2;
				k2 = 2 * k1;
				wk2r = w[k1];
				wk2i = w[k1 + 1];
				wk1r = w[k2];
				wk1i = w[k2 + 1];
				wk3r = wk1r - 2 * wk2i * wk1i;
				wk3i = 2 * wk2i * wk1r - wk1i;
				for (j = k; j < l + k; j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = a[j + 1] + a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = a[j + 1] - a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i + x2i;
					x0r -= x2r;
					x0i -= x2i;
					a[j2] = wk2r * x0r - wk2i * x0i;
					a[j2 + 1] = wk2r * x0i + wk2i * x0r;
					x0r = x1r - x3i;
					x0i = x1i + x3r;
					a[j1] = wk1r * x0r - wk1i * x0i;
					a[j1 + 1] = wk1r * x0i + wk1i * x0r;
					x0r = x1r + x3i;
					x0i = x1i - x3r;
					a[j3] = wk3r * x0r - wk3i * x0i;
					a[j3 + 1] = wk3r * x0i + wk3i * x0r;
				}
				wk1r = w[k2 + 2];
				wk1i = w[k2 + 3];
				wk3r = wk1r - 2 * wk2r * wk1i;
				wk3i = 2 * wk2r * wk1r - wk1i;
				for (j = k + m; j < l + (k + m); j += 2) {
					j1 = j + l;
					j2 = j1 + l;
					j3 = j2 + l;
					x0r = a[j] + a[j1];
					x0i = a[j + 1] + a[j1 + 1];
					x1r = a[j] - a[j1];
					x1i = a[j + 1] - a[j1 + 1];
					x2r = a[j2] + a[j3];
					x2i = a[j2 + 1] + a[j3 + 1];
					x3r = a[j2] - a[j3];
					x3i = a[j2 + 1] - a[j3 + 1];
					a[j] = x0r + x2r;
					a[j + 1] = x0i + x2i;
					x0r -= x2r;
					x0i -= x2i;
					a[j2] = -wk2i * x0r - wk2r * x0i;
					a[j2 + 1] = -wk2i * x0i + wk2r * x0r;
					x0r = x1r - x3i;
					x0i = x1i + x3r;
					a[j1] = wk1r * x0r - wk1i * x0i;
					a[j1 + 1] = wk1r * x0i + wk1i * x0r;
					x0r = x1r + x3i;
					x0i = x1i - x3r;
					a[j3] = wk3r * x0r - wk3i * x0i;
					a[j3 + 1] = wk3r * x0i + wk3i * x0r;
				}
			}
		}

		static inline void rftfsub(int n, T* a, int nc, T* c) {
			int j, k, kk, ks, m;
			T wkr, wki, xr, xi, yr, yi;
			m = n >> 1;
			ks = 2 * nc / m;
			kk = 0;
			for (j = 2; j < m; j += 2) {
				k = n - j;
				kk += ks;
				wkr = 0.5 - c[nc - kk];
				wki = c[kk];
				xr = a[j] - a[k];
				xi = a[j + 1] + a[k + 1];
				yr = wkr * xr - wki * xi;
				yi = wkr * xi + wki * xr;
				a[j] -= yr;
				a[j + 1] -= yi;
				a[k] += yr;
				a[k + 1] -= yi;
			}
		}

		static inline void rftbsub(int n, T* a, int nc, T* c) {
			int j, k, kk, ks, m;
			T wkr, wki, xr, xi, yr, yi;
			a[1] = -a[1];
			m = n >> 1;
			ks = 2 * nc / m;
			kk = 0;
			for (j = 2; j < m; j += 2) {
				k = n - j;
				kk += ks;
				wkr = 0.5 - c[nc - kk];
				wki = c[kk];
				xr = a[j] - a[k];
				xi = a[j + 1] + a[k + 1];
				yr = wkr * xr + wki * xi;
				yi = wkr * xi - wki * xr;
				a[j] -= yr;
				a[j + 1] = yi - a[j + 1];
				a[k] += yr;
				a[k + 1] = yi - a[k + 1];
			}
			a[m + 1] = -a[m + 1];
		}
	};

	using FFTOouraFloat = FFTOouraTpl<float>;
	using FFTOouraDouble = FFTOouraTpl<double>;

	// Default type
	#ifdef TKLB_SAMPLE_FLOAT
		using FFTOoura = FFTOouraTpl<float>;
	#else
		using FFTOoura = FFTOouraTpl<double>;
	#endif
} // namespace

#endif // _TKLB_FFT_OOURA
