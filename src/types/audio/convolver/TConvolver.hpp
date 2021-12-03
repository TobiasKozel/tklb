#ifndef _TKLB_CONVOLVER
#define _TKLB_CONVOLVER

/**
 * Using template interfaces isn't an option
 * so this simply defines a default Convolver type.
 * An unified interface is only enforced by the unit tests.
 */
#ifdef TKLB_CONVOLVER_BRUTE
	#include "./TConvolverBrute.hpp"
#endif
#ifdef TKLB_CONVOLVER_REF
	#include "./TConvolverRef.hpp"
#endif
#ifdef TKLB_CONVOLVER_FFT
	#include "./TConvolverFFT.hpp"
#endif

namespace tklb {
	template <typename T>
	#ifdef TKLB_CONVOLVER_BRUTE
		using ConvolverTpl = ConvolverBruteTpl<T>;
		using Convolver = ConvolverBrute;
	#endif
	#ifdef TKLB_CONVOLVER_REF
		using ConvolverTpl = ConvolverRefTpl<T>;
		using Convolver = ConvolverRef;
	#endif
	#ifdef TKLB_CONVOLVER_FFT
		// TODO tklb
		using ConvolverTpl = ConvolverRefTpl<T>;
		using Convolver = ConvolverRef;
	#endif
} // namespace

#endif // _TKLB_CONVOLVER
