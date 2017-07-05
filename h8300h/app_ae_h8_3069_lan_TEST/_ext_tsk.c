/*
 * _ext_tsk.c : ext_tsk/exd_tsk test
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
#if (TESTPTN & 4)

#include "kernel.h"
#include "tprintf.h"
#include "test.h"

static volatile int ext_mode;
static T_RMPL rmpl;

static TASK task1(VP_INT exinf)
{
    printf("    task1\n");
    if (ext_mode == 1) {
        ext_tsk();
    } else if (ext_mode == 2) {
        exd_tsk();
    }
}

const T_CTSK ctsk1 = {TA_HLNG | TA_STA, (VP_INT)0, task1, 1,  512};

static long stk[512/4];
static T_CTSK ctsk2 = {TA_HLNG | TA_STA, (VP_INT)0, task1, 1,  512, stk};

void _ext_tsk()
{
    SIZE chk;
    T_RTST rtst;

    printf("ext_tsk/exd_tsk test:\n");

    ref_mpl(-4, &rmpl);
    chk = rmpl.fmplsz;

    ext_mode = 0;               /* exit by return from task function */
    cre_tsk(2, &ctsk1);
    ref_tst(2, &rtst);
    if (rtst.tskstat != TTS_DMT) STOP(0);
    del_tsk(2);
    
    ext_mode = 1;               /* exit by ext_tsk */
    cre_tsk(2, &ctsk1);
    ref_tst(2, &rtst);
    if (rtst.tskstat != TTS_DMT) STOP(0);
    del_tsk(2);
    
    ext_mode = 2;               /* exit and delete by exd_tsk */
    cre_tsk(2, &ctsk1);
    if (_kernel_tcb[0]) STOP(0);
    
    ext_mode = 0;               /* exit by return from task function */
    cre_tsk(2, &ctsk2);
    ref_tst(2, &rtst);
    if (rtst.tskstat != TTS_DMT) STOP(0);
    del_tsk(2);
    
    ref_mpl(-4, &rmpl);
    if (chk != rmpl.fmplsz) STOP(0);

    printf("    finish\n\n");
}
#endif

/* end */
