#ifndef _TKLB_RESAMPLER
#define _TKLB_RESAMPLER

/**
 * Using template interfaces isn't an option
 * so this simply defines a default Resampler type.
 * An unified interface is only enforced by the unit tests.
 */
#ifdef TKLB_RESAMPLER_LINEAR
	#include "./TResamplerLinear.hpp"
#else
	#include "./TResamplerSpeex.hpp"
#endif

namespace tklb {
	template <typename T>
	#ifdef TKLB_RESAMPLER_LINEAR
		using ResamplerTpl = ResamplerLinearTpl<T>;
		using Resampler = ResamplerLinear;
	#else
		using ResamplerTpl = ResamplerSpeexTpl<T>;
		using Resampler = ResamplerSpeex;
	#endif
} // namespace

#endif // _TKLB_RESAMPLER
