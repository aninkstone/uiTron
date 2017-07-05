/*
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

/*
 * pesudo interrupt vector
 */
T_VECT _kernel_vect[_KERNEL_IRQ_CNT];

/*
 * def_inh system call
 */
ER def_inh(INHNO inhno, const T_DINH *pk_dinh)
{
    UINT psw;
    ER r = E_OK;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (inhno < 0 || _KERNEL_IRQ_CNT <= inhno
        || !pk_dinh || !pk_dinh->inthdr)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * set vector table
     */
    _kernel_vect[inhno].inthdr = (FP_INH)pk_dinh->inthdr;
#ifdef _KERNEL_INT_IMASK
    _kernel_vect[inhno].imask  = pk_dinh->imask;
#endif
    /*
     * end of critical section
     */
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DEF_INH();
    return r;
}

/* end */
