#ifndef TKLBZ_CONVOLVER
#define TKLBZ_CONVOLVER

/**
 * Using template interfaces isn't an option
 * so this simply defines a default Convolver type.
 * An unified interface is only enforced by the unit tests.
 */
#ifdef TKLB_CONVOLVER_BRUTE
	#include "./TConvolverBrute.hpp"
#else
	#include "./TConvolverRef.hpp"
#endif

namespace tklb {
	template <typename T>
	#ifdef TKLB_CONVOLVER_BRUTE
		using ConvolverTpl = ConvolverBruteTpl<T>;
		using Convolver = ConvolverBrute;
	#else
		using ConvolverTpl = ConvolverRefTpl<T>;
		using Convolver = ConvolverRef;
	#endif
} // namespace

#endif // TKLB_CONVOLVER
