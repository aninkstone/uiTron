/*
 * _cre_tsk.c : cre_tsk/acre_tsk/del_tsk/sta_tsk test
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
#if (TESTPTN & 2)

#include "kernel.h"
#include "tprintf.h"
#include "test.h"

static volatile VP_INT _exinf;
static char stk[512];
static volatile int cnt;
static volatile T_TCB *next;
static T_RMPL rmpl;

static TASK task1(VP_INT exinf)
{
    printf("    task1\n");
    _exinf = exinf;
    cnt++;
    next = _kernel_cur->n;
    slp_tsk();
}

static const T_CTSK ctsk1 = {TA_HLNG | TA_STA, (VP_INT)11, task1, 2,  512};
static       T_CTSK ctsk2 = {TA_HLNG         , (VP_INT)22, task1, 2,  512};

void _cre_tsk()
{
    T_TCB *tcb;
    SIZE chk;

    printf("cre_tsk/del_tsk/sta_tsk test:\n");

    ref_mpl(-4, &rmpl);
    chk = rmpl.fmplsz;

    cre_tsk(2, &ctsk1);
    if (cnt != 1) STOP(NULL);
    if (_exinf != (VP_INT)11) STOP(NULL);
    tcb = _kernel_tcb[1];
    if ((((T_KERBLK *)tcb)-1)->len !=
        sizeof(T_KERBLK) + TSZ_PTR(sizeof(T_TCB))) STOP(0);
    if (tcb->ctsk != &ctsk1) STOP(0);
    if ((char *)tcb->stk < _KERNEL_STKTOP()) STOP(0);
    if (next != (T_TCB *)&_kernel_rdq[1]) STOP(0);
    wup_tsk(2);
    del_tsk(2);

    cre_tsk(2, &ctsk2);
    if (_exinf != (VP_INT)11) STOP(NULL);
    sta_tsk(2, (VP_INT)22);
    if (cnt != 2) STOP(NULL);
    if (_exinf != (VP_INT)22) STOP(NULL);
    tcb = _kernel_tcb[1];
    if ((((T_KERBLK *)tcb)-1)->len !=
        sizeof(T_KERBLK) + TSZ_PTR(sizeof(T_TCB) + sizeof(T_CTSK))) STOP(0);
    if (tcb->ctsk != (T_CTSK *)(tcb + 1)) STOP(0);
    if ((char *)tcb->stk < _KERNEL_STKTOP()) STOP(0);
    wup_tsk(2);
    del_tsk(2);

    {
        T_CTSK ctsk3;
        ctsk3.tskatr = TA_HLNG;
        ctsk3.exinf = (VP_INT)33;
        ctsk3.task = task1;
        ctsk3.itskpri = 1;
        ctsk3.stksz = 512;
        ctsk3.stk = stk;
        ctsk3.name = NULL;
        cre_tsk(2, &ctsk3);
        act_tsk(2);
        if (cnt != 3) STOP(NULL);
        if (_exinf != (VP_INT)33) STOP(NULL);
        tcb = _kernel_tcb[1];
        if ((((T_KERBLK *)tcb)-1)->len !=
            sizeof(T_KERBLK) + TSZ_PTR(sizeof(T_TCB) + sizeof(T_CTSK))) STOP(0);
        if (tcb->ctsk != (T_CTSK *)(tcb + 1)) STOP(0);
        if (tcb->stk != stk) STOP(0);
        wup_tsk(2);
        del_tsk(2);
    }

    ctsk2.itskpri = 7;
    if (acre_tsk(&ctsk2) != 7) STOP(0);
    sta_tsk(7, (VP_INT)0);
    tcb = _kernel_tcb[7 - 1];
    if (cnt != 4) STOP(NULL);
    if (next != (T_TCB *)&_kernel_rdq[7 - 1]) STOP(0);
    wup_tsk(7);
    del_tsk(7);

    ref_mpl(-4, &rmpl);
    if (chk != rmpl.fmplsz) STOP(0);

    printf("    finish\n\n");
}
#endif

/* end */
