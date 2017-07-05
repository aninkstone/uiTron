/*
 * main.c : initialize apprication
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

#if 0
#define ISTKSZ 0x300
#define TSTKSZ 0x400
#define SYSMSZ 0x1000
#define MPLMSZ 0x2000
#define STKMSZ 0x3000
#endif

#define CYCNO_MAX 8
#define ALMNO_MAX 8
#include "config.h"
#include "tprintf.h"
#include "testptn.h"
#include "test.h"

void stop(char *file, int line, char *msg)
{
    t_printf("TEST ERROR\n%s:%d:%s.\n\n", file, line, msg? msg : "");
    _KERNEL_TRAP_GDB();
}

TASK init(VP_INT exinf)
{
    printf("START KERNEL TEST\n\n");
#if (TESTPTN & 1)
    _alloc();
#endif
#if (TESTPTN & 2)
    _cre_tsk();
#endif
#if (TESTPTN & 4)
    _ext_tsk();
#endif
#if (TESTPTN & 8)
    _act_tsk();
#endif
    printf("FINISH KERNEL TEST\n\n");
    _KERNEL_TRAP_GDB();
}

const T_CTSK ctsk = {TA_HLNG | TA_STA, (VP_INT)0, init, 8, 512};

int main()
{
    sysini();
    cre_tsk(8, &ctsk);
    intsta();
    syssta();
}

/* end */
