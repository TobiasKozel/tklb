/*****************************************************************************

        Downsampler2x8F64Avx512.hpp
        Author: Laurent de Soras, 2020

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_Downsampler2x8F64Avx512_CODEHEADER_INCLUDED)
#define hiir_Downsampler2x8F64Avx512_CODEHEADER_INCLUDED



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "./StageProc8F64Avx512.h"

#include <cassert>



namespace hiir
{



/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



template <int NC>
constexpr int 	Downsampler2x8F64Avx512 <NC>::_nbr_chn;
template <int NC>
constexpr int 	Downsampler2x8F64Avx512 <NC>::NBR_COEFS;
template <int NC>
constexpr double	Downsampler2x8F64Avx512 <NC>::_delay;



/*
==============================================================================
Name: ctor
Throws: Nothing
==============================================================================
*/

template <int NC>
Downsampler2x8F64Avx512 <NC>::Downsampler2x8F64Avx512 () noexcept
:	_filter ()
{
	for (int i = 0; i < NBR_COEFS + 2; ++i)
	{
		_mm512_store_pd (_filter [i]._coef, _mm512_setzero_pd ());
	}

	clear_buffers ();
}



/*
==============================================================================
Name: set_coefs
Description:
	Sets filter coefficients. Generate them with the PolyphaseIir2Designer
	class.
	Call this function before doing any processing.
Input parameters:
	- coef_arr: Array of coefficients. There should be as many coefficients as
		mentioned in the class template parameter.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2x8F64Avx512 <NC>::set_coefs (const double coef_arr []) noexcept
{
	assert (coef_arr != nullptr);

	for (int i = 0; i < NBR_COEFS; ++i)
	{
		_mm512_store_pd (
			_filter [i + 2]._coef, _mm512_set1_pd (DataType (coef_arr [i]))
		);
	}
}



/*
==============================================================================
Name: process_sample
Description:
	Downsamples (x2) one pair of vector of 8 samples, to generate one output
	vector.
Input parameters:
	- in_ptr: pointer on the two vectors to decimate. No alignment constraint.
Returns: Samplerate-reduced vector.
Throws: Nothing
==============================================================================
*/

template <int NC>
__m512d	Downsampler2x8F64Avx512 <NC>::process_sample (const double in_ptr [_nbr_chn * 2]) noexcept
{
	assert (in_ptr != nullptr);

	const __m512d   in_0 = _mm512_loadu_pd (in_ptr           );
	const __m512d   in_1 = _mm512_loadu_pd (in_ptr + _nbr_chn);

	return process_sample (in_0, in_1);
}



/*
==============================================================================
Name: process_sample
Description:
	Downsamples (x2) one pair of vector of 8 samples, to generate one output
	vector.
Input parameters:
	- in_0: vector at t
	- in_1: vector at t + 1
Returns: Samplerate-reduced vector.
Throws: Nothing
==============================================================================
*/

template <int NC>
__m512d	Downsampler2x8F64Avx512 <NC>::process_sample (__m512d in_0, __m512d in_1) noexcept
{
	__m512d         spl_0 = in_1;
	__m512d         spl_1 = in_0;

	StageProc8F64Avx512 <NBR_COEFS>::process_sample_pos (
		NBR_COEFS, spl_0, spl_1, &_filter [0]
	);

	const __m512d   sum = _mm512_add_pd (spl_0, spl_1);
	const __m512d   out = _mm512_mul_pd (sum, _mm512_set1_pd (0.5f));

	return out;
}



/*
==============================================================================
Name: process_block
Description:
	Downsamples (x2) a block of vectors of 8 samples.
	Input and output blocks may overlap, see assert() for details.
Input parameters:
	- in_ptr: Input array, containing nbr_spl * 2 vectors.
		No alignment constraint.
	- nbr_spl: Number of vectors to output, > 0
Output parameters:
	- out_ptr: Array for the output vectors, capacity: nbr_spl vectors.
		No alignment constraint.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2x8F64Avx512 <NC>::process_block (double out_ptr [], const double in_ptr [], long nbr_spl) noexcept
{
	assert (in_ptr  != nullptr);
	assert (out_ptr != nullptr);
	assert (out_ptr <= in_ptr || out_ptr >= in_ptr + nbr_spl * (_nbr_chn * 2));
	assert (nbr_spl > 0);

	long           pos = 0;
	do
	{
		const __m512d   val = process_sample (in_ptr + pos * (_nbr_chn * 2));
		_mm512_storeu_pd (out_ptr + pos * _nbr_chn, val);
		++ pos;
	}
	while (pos < nbr_spl);
}



/*
==============================================================================
Name: process_sample_split
Description:
	Split (spectrum-wise) in half a pair of vector of 8 samples. The lower part
	of the spectrum is a classic downsampling, equivalent to the output of
	process_sample().
	The higher part is the complementary signal: original filter response
	is flipped from left to right, becoming a high-pass filter with the same
	cutoff frequency. This signal is then critically sampled (decimation by 2),
	flipping the spectrum: Fs/4...Fs/2 becomes Fs/4...0.
Input parameters:
	- in_ptr: pointer on the pair of input vectors. No alignment constraint.
Output parameters:
	- low: output vector, lower part of the spectrum (downsampling)
	- high: output vector, higher part of the spectrum.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2x8F64Avx512 <NC>::process_sample_split (__m512d &low, __m512d &high, const double in_ptr [_nbr_chn * 2]) noexcept
{
	assert (in_ptr != nullptr);

	const __m512d   in_0 = _mm512_loadu_pd (in_ptr           );
	const __m512d   in_1 = _mm512_loadu_pd (in_ptr + _nbr_chn);

	process_sample_split (low, high, in_0, in_1);
}



/*
==============================================================================
Name: process_sample_split
Description:
	Split (spectrum-wise) in half a pair of vector of 8 samples. The lower part
	of the spectrum is a classic downsampling, equivalent to the output of
	process_sample().
	The higher part is the complementary signal: original filter response
	is flipped from left to right, becoming a high-pass filter with the same
	cutoff frequency. This signal is then critically sampled (decimation by 2),
	flipping the spectrum: Fs/4...Fs/2 becomes Fs/4...0.
Input parameters:
	- in_0: vector at t
	- in_1: vector at t + 1
Output parameters:
	- low: output vector, lower part of the spectrum (downsampling)
	- high: output vector, higher part of the spectrum.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2x8F64Avx512 <NC>::process_sample_split (__m512d &low, __m512d &high, __m512d in_0, __m512d in_1) noexcept
{
	__m512d         spl_0 = in_1;
	__m512d         spl_1 = in_0;

	StageProc8F64Avx512 <NBR_COEFS>::process_sample_pos (
		NBR_COEFS, spl_0, spl_1, &_filter [0]
	);

	const __m512d   sum = _mm512_add_pd (spl_0, spl_1);
	low  = _mm512_mul_pd (sum, _mm512_set1_pd (0.5f));
	high = _mm512_sub_pd (spl_0, low);
}



/*
==============================================================================
Name: process_block_split
Description:
	Split (spectrum-wise) in half a pair of vector of 8 samples. The lower part
	of the spectrum is a classic downsampling, equivalent to the output of
	process_block().
	The higher part is the complementary signal: original filter response
	is flipped from left to right, becoming a high-pass filter with the same
	cutoff frequency. This signal is then critically sampled (decimation by 2),
	flipping the spectrum: Fs/4...Fs/2 becomes Fs/4...0.
	Input and output blocks may overlap, see assert() for details.
Input parameters:
	- in_ptr: Input array, containing nbr_spl * 2 vectors.
		No alignment constraint.
	- nbr_spl: Number of vectors for each output, > 0
Output parameters:
	- out_l_ptr: Array for the output vectors, lower part of the spectrum
		(downsampling). Capacity: nbr_spl vectors.
		No alignment constraint.
	- out_h_ptr: Array for the output vectors, higher part of the spectrum.
		Capacity: nbr_spl vectors.
		No alignment constraint.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2x8F64Avx512 <NC>::process_block_split (double out_l_ptr [], double out_h_ptr [], const double in_ptr [], long nbr_spl) noexcept
{
	assert (in_ptr    != nullptr);
	assert (out_l_ptr != nullptr);
	assert (out_l_ptr <= in_ptr || out_l_ptr >= in_ptr + nbr_spl * (_nbr_chn * 2));
	assert (out_h_ptr != nullptr);
	assert (out_h_ptr <= in_ptr || out_h_ptr >= in_ptr + nbr_spl * (_nbr_chn * 2));
	assert (out_h_ptr != out_l_ptr);
	assert (nbr_spl > 0);

	long           pos = 0;
	do
	{
		__m512d         low;
		__m512d         high;
		process_sample_split (low, high, in_ptr + pos * (_nbr_chn * 2));
		_mm512_storeu_pd (out_l_ptr + pos * _nbr_chn, low);
		_mm512_storeu_pd (out_h_ptr + pos * _nbr_chn, high);
		++ pos;
	}
	while (pos < nbr_spl);
}



/*
==============================================================================
Name: clear_buffers
Description:
	Clears filter memory, as if it processed silence since an infinite amount
	of time.
Throws: Nothing
==============================================================================
*/

template <int NC>
void	Downsampler2x8F64Avx512 <NC>::clear_buffers () noexcept
{
	for (int i = 0; i < NBR_COEFS + 2; ++i)
	{
		_mm512_store_pd (_filter [i]._mem, _mm512_setzero_pd ());
	}
}



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



}  // namespace hiir



#endif   // hiir_Downsampler2x8F64Avx512_CODEHEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
