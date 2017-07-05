/*
 * timer.c : Kernel timer functions for H8/300H
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

#include "kernel.h"
#ifdef H8_3069
# include "h8_3069.h"
#endif

#define IMASK 0x8000            /* I=1,UI=0 : low  level interrupt */
/*#define IMASK 0xc000*/        /* I=1,UI=1 : high level interrupt */

#if !defined(CH) || CH == 0
#  define TCR8     TCR01_8
#  define TCSR8    TCSR01_8
#  define TCNT8    TCNT01_8
#  define TCORA8   TCORA01_8
#  define TCNT8    TCNT01_8
#  define INT_CMI8 INT_CMIA0
#elif CH == 1
#  define TCR8     TCR23_8
#  define TCSR8    TCSR23_8
#  define TCNT8    TCNT23_8
#  define TCORA8   TCORA23_8
#  define TCNT8    TCNT23_8
#  define INT_CMI8 INT_CMIA2
#else
#  error bad CH
#endif

static ID isrid _KERNEL_DATA = 0;

static void tim_int(INHNO irq)
{
    *(UH *)0 = TCSR8;
    TCSR8 = 0x0000;
    isig_tim();
}

ER intsta(void)
{
    ER r = E_OK;

#ifdef _KERNEL_OVR
    static const T_CISR cisr = {TA_HLNG, 0, INT_CMI8, (FP)tim_int, IMASK};
    if ((r = acre_isr(&cisr)) < 0)
        goto ret;
    isrid = (ID)r;
#else /* _KERNEL_OVR */
    static const T_DINH dinh = {TA_HLNG, (FP)tim_int, IMASK};
    if ((r = def_inh(INT_CMI8, &dinh)) != E_OK)
        goto ret;
#endif /* _KERNEL_OVR */

    TCR8   = 0x0000;            /* stop */
    *(UH *)0 = TCSR8;
    TCSR8  = 0x0000;
    TCORA8 = 25000U;            /* 20MHz - 10ms */
    TCR8   = 0x4c01;            /* 16bit counter mode */
ret:
    _KERNEL_ASSERT_INTSTA();
    return r;
}

ER intext(void)
{
    ER r = E_OK;

    TCR8   = 0x0000;            /* stop */
    if (isrid)
        r = del_isr(isrid);
    _KERNEL_ASSERT_INTEXT();
    return r;
}

/* end */
