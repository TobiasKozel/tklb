#ifndef TKLBZ_FFT
#define TKLBZ_FFT

// TODO tklb Turn into interface

#ifdef TKLB_USE_OOURA
	#include "./TOouraFFT.hpp"
namespace tklb {
	using FFT = FFTOoura;
}
#else
	#include "./Tpffft.hpp"
namespace tklb {
	using FFT = FFTpffft;
}
#endif

#endif // TKLB_FFT
