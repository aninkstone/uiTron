/*
 * task1.c : task1
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

#include "kernel.h"             /* system header */
#include "tprintf.h"            /* tiny printf header */

TASK task1(VP_INT exinf)
{
    printf("task1 started. (exinf=%d)\n", exinf);
    for (;;) {
        printf("task1 slept and gave CPU.\n");
        slp_tsk();              /* sleep until waked up */
        printf("task1 was waken up and got CPU.\n");
    }
}

/* end */
