/*
 * _alloc.c : kernel alloc func test
 *
 * Copyright (C) 2007,2008,2009,2010
 * 256TECH Co., Ltd.
 * Masahiro Sakamoto (m-sakamoto@users.sourceforge.net)
 *
 * This file is part of URIBO.
 *
 * URIBO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * URIBO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with URIBO; see the file COPYING and COPYING.LESSER.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "testptn.h"
#if (TESTPTN & 1)

#include "kernel.h"
#include "tprintf.h"
#include "test.h"

static T_RMPL rmpl;

static void ref()
{
    ref_mpl(-4, &rmpl);
    printf("    ref_mpl: id=%d, total=0x%x, max=0x%x\n",
           rmpl.wtskid, rmpl.fmplsz, rmpl.fblksz);
}

static char *ptr[256];

void _alloc()
{
    int i, cnt;
    SIZE s;
    char *p;
    SIZE chk;

    printf("kernel alloc func test:\n");
    ref();
    chk = rmpl.fmplsz;
    printf("    alloc x256\n");
    for (i = 0; i < 255; i++) {
        s = rand() & 0xfff;
        p = _kernel_stk_alloc(s);
        if (!p) STOP("Too many alloc, reduce alloc size mask");
        ptr[i] = p;
    }
    ref_mpl(-4, &rmpl);
    s = rmpl.fmplsz;
    p = _kernel_stk_alloc(s);
    if (!p) STOP("alloc 1");
    ptr[255] = p;
    ref();
    if (rmpl.fmplsz) STOP("alloc 2");

    printf("    free x128\n");
    for (cnt = 0; cnt < 128; ) {
        i = rand() & 255;
        p = ptr[i];
        if (p) {
            _kernel_stk_free(p);
            ptr[i] = NULL;
            cnt++;
        }
    }
    ref();

    printf("    re-alloc x128\n");
    for (cnt = 0; cnt < 128; ) {
        i = rand() & 255;
        p = ptr[i];
        if (!p) {
            s = rand() & 0xff;
            p = _kernel_stk_alloc(s);
            if (p) {
                ptr[i] = p;
                cnt++;
            }
        }
    }
    ref();

    printf("    re-free x256\n");
    for (cnt = 0; cnt < 256; ) {
        i = rand() & 255;
        p = ptr[i];
        if (p) {
            _kernel_stk_free(p);
            ptr[i] = NULL;
            cnt++;
        }
    }
    ref();

    if (rmpl.fmplsz != chk) STOP("alloc 3");
    printf("    finish\n\n");
}
#endif

/* end */
