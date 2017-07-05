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
 * sta_tsk system call
 */
ER sta_tsk(ID tskid, VP_INT stacd)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;

#ifndef _KERNEL_NO_STATIC_ERR
    {
        UB s = _kernel_sts;
        /*
         * check context
         */
        if (_KERNEL_CHK_LOC(s))
            _KERNEL_RET(E_CTX);
        /*
         * check par
         */
        if (tskid == TSK_SELF) {
            _KERNEL_RET((_KERNEL_CHK_IS(s))? E_ID : E_OBJ);
        } else if (_kernel_tskid_max < (UINT)tskid) {
            _KERNEL_RET(E_ID);
        } else if (_KERNEL_CHK_TSK(s) && tskid == _kernel_cur->id) {
            _KERNEL_RET(E_OBJ);
        }
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get TCB
     */
    if (!(tcb = _kernel_tcb[tskid - 1]))
        _KERNEL_END(E_NOEXS);
    /*
     * start task
     */
    if (tcb->sts != TTS_DMT)
        _KERNEL_END(E_OBJ);
    _kernel_act(tcb, stacd);
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_STA_TSK();
    return r;
}

/* end */
