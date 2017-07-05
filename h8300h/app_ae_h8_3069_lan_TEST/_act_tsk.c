/*
 * _act_tsk.c : act_tsk/can_act test
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
#if (TESTPTN & 8)

#ifndef _KERNEL_OVR
# error Please add CFLAGS to -D_KERNEL_OVR at Makefile
#endif

#include "kernel.h"
#include "tprintf.h"
#include "test.h"
#ifdef H8_3069
# include "h8_3069.h"
#endif

static volatile int cnt;
static volatile int cnt_isr_act, cnt_cyc_act, cnt_tsk_act;
static volatile int cnt_isr_can, cnt_cyc_can, cnt_tsk_can;
static T_RMPL rmpl;
static T_RTSK rtsk;
static volatile int sleep, cancel;

static void isrhdr1()
{
    for (; cnt_isr_act; cnt_isr_act--)
        act_tsk(3);
}

static void isrhdr2()
{
    for (; cnt_isr_can; cnt_isr_can--)
        can_act(3);
}

static void cychdr1()
{
    for (; cnt_cyc_act; cnt_cyc_act--)
        act_tsk(3);
}

static void cychdr2()
{
    static int chk = 60000/MSEC; /* TIME OUT */

    for (; cnt_cyc_can; cnt_cyc_can--)
        can_act(3);
    if (!--chk)
        STOP("_act_tsk hanged up");
}

static TASK task1()
{
    for (;;) {
        slp_tsk();
        for (; cnt_tsk_act; cnt_tsk_act--)
            act_tsk(3);
    }
}

static TASK task2()
{
    for (;;) {
        slp_tsk();
        for (; cnt_tsk_can; cnt_tsk_can--)
            can_act(3);
    }
}

static TASK task3()
{
    printf("    task3\n");
    cnt++;
    if (sleep)
        slp_tsk();
    if (cancel)
        can_act(TSK_SELF);
}

#ifdef H8_3069
static const T_CISR cisr1 = {TA_HLNG, (VP_INT)0, INT_CMIA0, isrhdr1, 0x8000};
static const T_CISR cisr2 = {TA_HLNG, (VP_INT)0, INT_CMIA0, isrhdr2, 0x8000};
#endif
static const T_CCYC ccyc1 = {TA_HLNG | TA_STA, (VP_INT)0, cychdr1, 1, 0};
static const T_CCYC ccyc2 = {TA_HLNG | TA_STA, (VP_INT)0, cychdr2, 1, 0};
static const T_CTSK ctsk1 = {TA_HLNG | TA_STA, (VP_INT)0, task1, 1,  512};
static const T_CTSK ctsk2 = {TA_HLNG | TA_STA, (VP_INT)0, task2, 2,  512};
static const T_CTSK ctsk3 = {TA_HLNG         , (VP_INT)0, task3, 3,  512};

void _act_tsk()
{
    T_TCB *tcb;
    SIZE chk;
    int i;
    VP chk_sp;

    printf("act_tsk test:\n");

    ref_mpl(-4, &rmpl);
    chk = rmpl.fmplsz;

    /* create objects */
    cre_isr(1, &cisr1);
    cre_isr(2, &cisr2);
    cre_cyc(1, &ccyc1);
    cre_cyc(2, &ccyc2);
    cre_tsk(1, &ctsk1);
    cre_tsk(2, &ctsk2);
    cre_tsk(3, &ctsk3);

    /* act_tsk from ISR (not queuing) */
    printf("    test1\n");
    for (i = 1; i <= 10; i++) {
        cnt_isr_act = 1;
        while (cnt != i);
    }

    /* act_tsk from ISR (queuing) */
    printf("    test2\n");
    cnt_isr_act = 10;
    while (cnt != 20);

    /* act_tsk from CYC (not queuing) */
    printf("    test3\n");
    for (i = 1; i <= 10; i++) {
        cnt_cyc_act = 1;
        while (cnt != 20 + i);
    }

    /* act_tsk from CYC (queuing) */
    printf("    test4\n");
    cnt_cyc_act = 10;
    while (cnt != 40);

    /* act_tsk from task (not queuing) */
    printf("    test5\n");
    for (i = 1; i <= 10; i++) {
        cnt_tsk_act = 1;
        wup_tsk(1);
        while (cnt != 40 + i);
    }

    /* act_tsk from task (queuing) */
    printf("    test6\n");
    cnt_tsk_act = 10;
    wup_tsk(1);
    while (cnt != 60);

    /* act_tsk from task (queuing) */
    printf("    test7\n");
    sleep = 1;
    cnt_tsk_act = 10;
    wup_tsk(1);
    dly_tsk(2);
    if (cnt != 61) STOP(0);
    sleep = 0;
    wup_tsk(3);
    while (cnt != 70);

    /* can_act from ISR */
    printf("    test8\n");
    sleep = 1;
    cnt_isr_act = 10;
    dly_tsk(2);
    if (cnt != 71) STOP(0);
    ref_tsk(3, &rtsk);
    if (rtsk.tskstat != TTS_WAI) STOP(0);
    if (rtsk.tskwait != TTW_SLP) STOP(0);
    if (rtsk.lefttmo != 0) STOP(0);
    if (rtsk.actcnt != 9) STOP(0);
    cnt_isr_can = 1;
    dly_tsk(2);
    wup_tsk(3);
    if (cnt != 71) STOP(0);
    
    /* can_act from CYC */
    printf("    test9\n");
    cnt_cyc_act = 10;
    dly_tsk(2);
    if (cnt != 72) STOP(0);
    cnt_cyc_can = 1;
    dly_tsk(2);
    wup_tsk(3);
    if (cnt != 72) STOP(0);
    
    /* can_act from task */
    printf("    test10\n");
    cnt_tsk_act = 10;
    wup_tsk(1);
    if (cnt != 73) STOP(0);
    cnt_tsk_can = 1;
    wup_tsk(2);
    if (cnt_tsk_can) STOP(0);
    sleep = 0;
    wup_tsk(3);
    if (cnt != 73) STOP(0);

    /* can_act from task onself */
    printf("    test11\n");
    sleep = 1;
    cnt_tsk_act = 10;
    wup_tsk(1);
    if (cnt != 74) STOP(0);
    sleep = 0;
    cancel = 1;
    wup_tsk(3);
    if (cnt != 74) STOP(0);

    /* fin */
    del_isr(1);
    del_isr(2);
    del_cyc(1);
    del_cyc(2);
    ter_tsk(1);
    del_tsk(1);
    ter_tsk(2);
    del_tsk(2);
    del_tsk(3);
    ref_mpl(-4, &rmpl);
    if (chk != rmpl.fmplsz) STOP(0);

    printf("    finish\n\n");
}
#endif

/* end */
