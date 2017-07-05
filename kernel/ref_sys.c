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
 * ref_sys system call
 */
ER ref_sys(ID id, T_RSYS *pk_rsys)
{
    UINT psw;
    ER r = E_OK;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check par
     */
    if (!pk_rsys)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get info
     */
    {
        INT sysstat;
        UB s = _kernel_sts;
        if (_KERNEL_CHK_IS(s)) {
            sysstat = TSS_INDP;
        } else if (_KERNEL_CHK_LOC(s)) {
            sysstat = TSS_LOC;
        } else if (_KERNEL_CHK_DSP(s)) {
            sysstat = TSS_DDSP;
        } else {
            sysstat = TSS_TSK;
        }
        pk_rsys->sysstat = sysstat;
    }
    /*
     * end of critical section
     */
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_REF_SYS();
    return r;
}

/* eof */
