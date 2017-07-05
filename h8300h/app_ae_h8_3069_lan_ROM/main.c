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

#include "kernel.h"             /* system header */
#include "config.h"             /* kernel configration header */

TASK task1(VP_INT);
TASK task2(VP_INT);

/*
 * task1 has 1st priority and task2 has 2nd.
 * So, task1 is higher than task2.
 *                    attribute         exinf       func   pri stack     */
const T_CTSK ctsk1 = {TA_HLNG | TA_STA, (VP_INT)11, task1, 1,  256, NULL, ""};
const T_CTSK ctsk2 = {TA_HLNG | TA_STA, (VP_INT)22, task2, 2,  256, NULL, ""};

int main()
{
    sysini();                   /* initialize system */
    cre_tsk(1, &ctsk1);         /* create task object */
    cre_tsk(2, &ctsk2);         /* create task object */
    intsta();                   /* start interrupt by system timer */
    syssta();                   /* start system */
}

/* end */
