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

    Copyright (C) 2012 Fredrik Johansson

******************************************************************************/

#include "fmprb_poly.h"

/* This gives some speedup for small lengths. */
static __inline__ void
_fmprb_poly_rem_2(fmprb_struct * r, const fmprb_struct * a, long al,
    const fmprb_struct * b, long bl, long prec)
{
    if (al == 2)
    {
        fmprb_mul(r + 0, a + 1, b + 0, prec);
        fmprb_sub(r + 0, a + 0, r + 0, prec);
    }
    else
    {
        _fmprb_poly_rem(r, a, al, b, bl, prec);
    }
}

void
_fmprb_poly_evaluate_vec_fast_precomp(fmprb_struct * vs, const fmprb_struct * poly,
    long plen, fmprb_struct ** tree, long len, long prec)
{
    long height, i, j, pow, left;
    long tree_height;
    long tlen;
    fmprb_struct *t, *u, *swap, *pa, *pb, *pc;

    /* avoid worrying about some degenerate cases */
    if (len < 2 || plen < 2)
    {
        if (len == 1)
        {
            fmprb_t tmp;
            fmprb_init(tmp);
            fmprb_neg(tmp, tree[0] + 0);
            _fmprb_poly_evaluate(vs + 0, poly, plen, tmp, prec);
            fmprb_clear(tmp);
        }
        else if (len != 0 && plen == 0)
        {
            _fmprb_vec_zero(vs, len);
        }
        else if (len != 0 && plen == 1)
        {
            for (i = 0; i < len; i++)
                fmprb_set(vs + i, poly + 0);
        }
        return;
    }

    t = _fmprb_vec_init(len);
    u = _fmprb_vec_init(len);

    left = len;

    /* Initial reduction. We allow the polynomial to be larger
        or smaller than the number of points. */
    height = FLINT_BIT_COUNT(plen - 1) - 1;
    tree_height = FLINT_CLOG2(len);
    while (height >= tree_height)
        height--;
    pow = 1L << height;

    for (i = j = 0; i < len; i += pow, j += (pow + 1))
    {
        tlen = ((i + pow) <= len) ? pow : len % pow;
        _fmprb_poly_rem(t + i, poly, plen, tree[height] + j, tlen + 1, prec);
    }

    for (i = height - 1; i >= 0; i--)
    {
        pow = 1L << i;
        left = len;
        pa = tree[i];
        pb = t;
        pc = u;

        while (left >= 2 * pow)
        {
            _fmprb_poly_rem_2(pc, pb, 2 * pow, pa, pow + 1, prec);
            _fmprb_poly_rem_2(pc + pow, pb, 2 * pow, pa + pow + 1, pow + 1, prec);

            pa += 2 * pow + 2;
            pb += 2 * pow;
            pc += 2 * pow;
            left -= 2 * pow;
        }

        if (left > pow)
        {
            _fmprb_poly_rem(pc, pb, left, pa, pow + 1, prec);
            _fmprb_poly_rem(pc + pow, pb, left, pa + pow + 1, left - pow + 1, prec);
        }
        else if (left > 0)
            _fmprb_vec_set(pc, pb, left);

        swap = t;
        t = u;
        u = swap;
    }

    _fmprb_vec_set(vs, t, len);
    _fmprb_vec_clear(t, len);
    _fmprb_vec_clear(u, len);
}

void _fmprb_poly_evaluate_vec_fast(fmprb_struct * ys, const fmprb_struct * poly, long plen,
    const fmprb_struct * xs, long n, long prec)
{
    fmprb_struct ** tree;

    tree = _fmprb_poly_tree_alloc(n);
    _fmprb_poly_tree_build(tree, xs, n, prec);
    _fmprb_poly_evaluate_vec_fast_precomp(ys, poly, plen, tree, n, prec);
    _fmprb_poly_tree_free(tree, n);
}

void
fmprb_poly_evaluate_vec_fast(fmprb_struct * ys,
        const fmprb_poly_t poly, const fmprb_struct * xs, long n, long prec)
{
    _fmprb_poly_evaluate_vec_fast(ys, poly->coeffs,
                                        poly->length, xs, n, prec);
}
