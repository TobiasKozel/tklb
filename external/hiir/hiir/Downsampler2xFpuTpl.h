/*****************************************************************************

        Downsampler2xFpuTpl.h
        Author: Laurent de Soras, 2005

Downsamples by a factor 2 the input signal, using FPU.

Template parameters:
	- NC: number of coefficients, > 0

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_Downsampler2xFpuTpl_HEADER_INCLUDED)
#define hiir_Downsampler2xFpuTpl_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

#include "hiir/def.h"

#include <array>



namespace hiir
{



template <int NC, typename DT>
class Downsampler2xFpuTpl
{

	static_assert ((NC > 0), "Number of coefficient must be positive.");

/*\\\ PUBLIC \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

public:

	typedef DT DataType;
	static const int  _nbr_chn = 1;

	enum {         NBR_COEFS = NC };

	               Downsampler2xFpuTpl ();
	               Downsampler2xFpuTpl (const Downsampler2xFpuTpl <NC, DT> &other) = default;
	               Downsampler2xFpuTpl (Downsampler2xFpuTpl <NC, DT> &&other) = default;
	               ~Downsampler2xFpuTpl ()                                = default;

	Downsampler2xFpuTpl <NC, DT> &
	               operator = (const Downsampler2xFpuTpl <NC, DT> &other) = default;
	Downsampler2xFpuTpl <NC, DT> &
	               operator = (Downsampler2xFpuTpl <NC, DT> &&other)      = default;

	void           set_coefs (const double coef_arr []);

	hiir_FORCEINLINE DataType
	               process_sample (const DataType in_ptr [2]);
	void           process_block (DataType out_ptr [], const DataType in_ptr [], long nbr_spl);

	hiir_FORCEINLINE void
	               process_sample_split (DataType &low, DataType &high, const DataType in_ptr [2]);
	void           process_block_split (DataType out_l_ptr [], DataType out_h_ptr [], const DataType in_ptr [], long nbr_spl);

	void           clear_buffers ();



/*\\\ PROTECTED \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

protected:



/*\\\ PRIVATE \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	typedef	std::array <DataType, NBR_COEFS>	HyperGluar;

	HyperGluar     _coef;
	HyperGluar     _x;
	HyperGluar     _y;



/*\\\ FORBIDDEN MEMBER FUNCTIONS \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

private:

	bool           operator == (const Downsampler2xFpuTpl <NC, DT> &other) = delete;
	bool           operator != (const Downsampler2xFpuTpl <NC, DT> &other) = delete;

}; // class Downsampler2xFpuTpl



}  // namespace hiir



#include "hiir/Downsampler2xFpuTpl.hpp"



#endif   // hiir_Downsampler2xFpuTpl_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
