/* minidjvu - library for handling bilevel images with DjVuBitonal support
 *
 * erosion.c - determine candidates for flipping by image encoder
 *
 * Copyright (C) 2005  Ilya Mezhirov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 
 * minidjvu is derived from DjVuLibre (http://djvu.sourceforge.net)
 * All over DjVuLibre there is a patent alert from LizardTech
 * which I guess I should reproduce (don't ask me what does this mean):
 * 
 *  ------------------------------------------------------------------
 * | DjVu (r) Reference Library (v. 3.5)
 * | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
 * | The DjVu Reference Library is protected by U.S. Pat. No.
 * | 6,058,214 and patents pending.
 * |
 * | This software is subject to, and may be distributed under, the
 * | GNU General Public License, Version 2. The license should have
 * | accompanied the software or you may obtain a copy of the license
 * | from the Free Software Foundation at http://www.fsf.org .
 * |
 * | The computer code originally released by LizardTech under this
 * | license and unmodified by other parties is deemed "the LIZARDTECH
 * | ORIGINAL CODE."  Subject to any third party intellectual property
 * | claims, LizardTech grants recipient a worldwide, royalty-free, 
 * | non-exclusive license to make, use, sell, or otherwise dispose of 
 * | the LIZARDTECH ORIGINAL CODE or of programs derived from the 
 * | LIZARDTECH ORIGINAL CODE in compliance with the terms of the GNU 
 * | General Public License.   This grant only confers the right to 
 * | infringe patent claims underlying the LIZARDTECH ORIGINAL CODE to 
 * | the extent such infringement is reasonably necessary to enable 
 * | recipient to make, have made, practice, sell, or otherwise dispose 
 * | of the LIZARDTECH ORIGINAL CODE (or portions thereof) and not to 
 * | any greater extent that may be necessary to utilize further 
 * | modifications or combinations.
 * |
 * | The LIZARDTECH ORIGINAL CODE is provided "AS IS" WITHOUT WARRANTY
 * | OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * | TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
 * | MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 * +------------------------------------------------------------------
 */


#include "mdjvucfg.h"
#include "minidjvu.h"
#include <stdlib.h>


/* ERROR ALERT:
 * We should never declare border pixels
 * as erosion candidates or otherwise bounding boxes may change
 * with unpredictable consequences.
 * (this implementation is OK)
 */

/*
 * it all resembles `smooth' algorithm very much.
 */

/*
 * This implementation permits flipping of a pixel
 * when it has exactly 2 of 4 edge neighbors black
 *         and exactly 2 of 4 vertex (diagonal) neighbors black.
 */


/* all input rows must be 0-or-1 unpacked */
static void get_erosion_candidates_in_a_row(
                       unsigned char *r, /* result    */
                       unsigned char *u, /* upper row */
                       unsigned char *t, /* this row */
                       unsigned char *l, /* lower row */
                       int32 n)
{
    int32 i;
    n--;
    r[0] = 0;
    r[n] = 0;
    for (i = 1; i < n; i++)
    {
        int score_4 = u[i] + l[i] + t[i-1] + t[i+1] - 2;
        int score_d = u[i-1] + l[i-1] + u[i+1] + l[i+1] - 2;

        r[i] = !score_4 && !score_d ? 1 : 0;
    }
}

MDJVU_IMPLEMENT mdjvu_bitmap_t mdjvu_get_erosion_mask(mdjvu_bitmap_t bmp)
{
    int32 w = mdjvu_bitmap_get_width(bmp);
    int32 h = mdjvu_bitmap_get_height(bmp);
    mdjvu_bitmap_t result = mdjvu_bitmap_create(w, h);
    int32 i;
    unsigned char *u, *t, *l, *r;

    if (h < 3) return result;

    u = (unsigned char *) malloc(w); /* upper row */
    t = (unsigned char *) malloc(w); /* this row */
    l = (unsigned char *) malloc(w); /* lower row */
    r = (unsigned char *) malloc(w); /* result */

    mdjvu_bitmap_unpack_row_0_or_1(bmp, t, 0);
    mdjvu_bitmap_unpack_row_0_or_1(bmp, l, 1);
    for (i = 1; i < h - 1; i++)
    {
        unsigned char *tmp = u;
        u = t;
        t = l;
        l = tmp;

        mdjvu_bitmap_unpack_row_0_or_1(bmp, l, i + 1);

        get_erosion_candidates_in_a_row(r, u, t, l, w);
        mdjvu_bitmap_pack_row(result, r, i);
    }

    free(u);
    free(t);
    free(l);
    free(r);

    return result;
}
