#ifndef _TKLB_FFT
#define _TKLB_FFT


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

#endif // _TKLB_FFT
