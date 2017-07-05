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
 * ras_tex/iras_tex system call
 */
ER ras_tex(ID tskid, TEXPTN rasptn)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    UB s = _kernel_sts;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(s))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (tskid == TSK_SELF) {
        if (_KERNEL_CHK_IS(s))
            _KERNEL_RET(E_ID);
    } else if (_kernel_tskid_max < (UINT)tskid) {
        _KERNEL_RET(E_ID);
    }
    if (!rasptn)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get TCB
     */
    if (tskid == TSK_SELF) {
        tcb = (T_TCB *)_kernel_cur;
    } else if (!(tcb = _kernel_tcb[tskid - 1])) {
        _KERNEL_END(E_NOEXS);
    }
    /*
     * update pattern
     */
    if (tcb->sts == TTS_DMT)
        _KERNEL_END(E_OBJ);
    {
        T_TEX *tex = (T_TEX *)tcb->tex; /* avoid volatile effect */
        if (!tex)
            _KERNEL_END(E_OBJ);         /* not defined yet */
        tex->ptn |= rasptn;             /* update tex pattern */
    }
    /*
     * call handler directly if it is current task
     */
    if (_KERNEL_CHK_TSK(s) && tcb == _kernel_cur)
        _kernel_tex(tcb, psw);  /* check and call */
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_RAS_TEX();
    return r;
}

/* end */
