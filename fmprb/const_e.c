/*=============================================================================

    This file is part of ARB.

    ARB is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    ARB is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ARB; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

=============================================================================*/
/******************************************************************************

    Copyright (C) 2013 Fredrik Johansson

******************************************************************************/

#include "fmprb.h"
#include "hypgeom.h"

void
fmprb_const_e_eval(fmprb_t s, long prec)
{
    hypgeom_t series;
    fmprb_t t;

    fmprb_init(t);
    hypgeom_init(series);

    fmpz_poly_set_str(series->A, "1  1");
    fmpz_poly_set_str(series->B, "1  1");
    fmpz_poly_set_str(series->P, "1  1");
    fmpz_poly_set_str(series->Q, "2  0 1");

    prec += FLINT_CLOG2(prec);
    fmprb_hypgeom_infsum(s, t, series, prec, prec);
    fmprb_div(s, s, t, prec);

    hypgeom_clear(series);
    fmprb_clear(t);
}

DEF_CACHED_CONSTANT(fmprb_const_e, fmprb_const_e_eval)

