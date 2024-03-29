/*****************************************************************************

        Upsampler2x8F64Avx512.h
        Author: Laurent de Soras, 2020

Upsamples vectors of 8 double by a factor 2 the input signal, using the
AVX-512 instruction set.

This object must be aligned on a 64-byte boundary!

Template parameters:
	- NC: number of coefficients, > 0

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#pragma once
#if ! defined (hiir_Upsampler2x8F64Avx512_HEADER_INCLUDED)
#define hiir_Upsampler2x8F64Avx512_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma warning (4 : 4250)
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "./def.h"
#include "./StageDataF64Avx512.h"

#include <immintrin.h> 

#include <array>



namespace hiir
{



template <int NC>
class Upsampler2x8F64Avx512
{

	static_assert ((NC > 0), "Number of coefficient must be positive.");

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef double DataType;
	static constexpr int _nbr_chn  = 8;
	static constexpr int NBR_COEFS = NC;
	static constexpr double _delay = 0;

	               Upsampler2x8F64Avx512 () noexcept;
	               Upsampler2x8F64Avx512 (const Upsampler2x8F64Avx512 <NC> &other) = default;
	               Upsampler2x8F64Avx512 (Upsampler2x8F64Avx512 <NC> &&other)      = default;
	               ~Upsampler2x8F64Avx512 ()                            = default;

	Upsampler2x8F64Avx512 <NC> &
	               operator = (const Upsampler2x8F64Avx512 <NC> &other) = default;
	Upsampler2x8F64Avx512 <NC> &
	               operator = (Upsampler2x8F64Avx512 <NC> &&other)      = default;

	void				set_coefs (const double coef_arr [NBR_COEFS]) noexcept;
	hiir_FORCEINLINE void
	               process_sample (__m512d &out_0, __m512d &out_1, __m512d input) noexcept;
	void				process_block (double out_ptr [], const double in_ptr [], long nbr_spl) noexcept;
	void				clear_buffers () noexcept;



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	typedef std::array <StageDataF64Avx512, NBR_COEFS + 2> Filter;   // Stages 0 and 1 contain only input memories

	Filter         _filter; // Should be the first member (thus easier to align)



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	bool           operator == (const Upsampler2x8F64Avx512 <NC> &other) const = delete;
	bool           operator != (const Upsampler2x8F64Avx512 <NC> &other) const = delete;

}; // class Upsampler2x8F64Avx512



}  // namespace hiir



#include "./Upsampler2x8F64Avx512.hpp"



#endif   // hiir_Upsampler2x8F64Avx512_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
