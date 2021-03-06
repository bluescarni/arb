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

    Copyright (C) 2013, 2014 Fredrik Johansson

******************************************************************************/

#include "fmpr.h"

#define MUL_STACK_ALLOC 40
#define MUL_TLS_ALLOC 1000

TLS_PREFIX mp_ptr __mul_tmp = NULL;
TLS_PREFIX long __mul_alloc = 0;

void _mul_tmp_cleanup(void)
{
    flint_free(__mul_tmp);
    __mul_tmp = NULL;
    __mul_alloc = 0;
}

#define MUL_TMP_ALLOC \
    if (alloc <= MUL_STACK_ALLOC) \
    { \
        tmp = tmp_stack; \
    } \
    else if (alloc <= MUL_TLS_ALLOC) \
    { \
        if (__mul_alloc < alloc) \
        { \
            if (__mul_alloc == 0) \
            { \
                flint_register_cleanup_function(_mul_tmp_cleanup); \
            } \
            __mul_tmp = flint_realloc(__mul_tmp, sizeof(mp_limb_t) * alloc); \
            __mul_alloc = alloc; \
        } \
        tmp = __mul_tmp; \
    } \
    else \
    { \
        tmp = flint_malloc(sizeof(mp_limb_t) * alloc); \
    }

#define MUL_TMP_FREE \
    if (alloc > MUL_TLS_ALLOC) \
        flint_free(tmp);

long
_fmpr_mul_mpn(fmpr_t z,
    mp_srcptr xman, mp_size_t xn, const fmpz_t xexp,
    mp_srcptr yman, mp_size_t yn, const fmpz_t yexp,
    int negative, long prec, fmpr_rnd_t rnd)
{
    long zn, alloc, ret, shift;
    mp_limb_t tmp_stack[MUL_STACK_ALLOC];
    mp_ptr tmp;

    zn = xn + yn;
    alloc = zn;

    MUL_TMP_ALLOC

    if (yn == 1)
    {
        mp_limb_t cy = mpn_mul_1(tmp, xman, xn, yman[0]);
        tmp[zn - 1] = cy;
        zn = zn - (cy == 0);
    }
    else
    {
        mpn_mul(tmp, xman, xn, yman, yn);
        zn = zn - (tmp[zn - 1] == 0);
    }

    ret = _fmpr_set_round_mpn(&shift, fmpr_manref(z), tmp, zn, negative, prec, rnd);
    fmpz_add2_fmpz_si_inline(fmpr_expref(z), xexp, yexp, shift);

    MUL_TMP_FREE

    return ret;
}

