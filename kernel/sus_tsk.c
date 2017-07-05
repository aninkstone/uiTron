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
 * sus_tsk system call
 */
ER sus_tsk(ID tskid)
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
            if (_KERNEL_CHK_IS(s))
                _KERNEL_RET(E_ID);
            if (_KERNEL_CHK_DSP(s))
                _KERNEL_RET(E_CTX);
        } else if (_kernel_tskid_max < (UINT)tskid) {
            _KERNEL_RET(E_ID);
        } else if (_KERNEL_CHK_DSP(s)) {
            T_TCB *cur = (T_TCB *)_kernel_cur; /* avoid volatile effect */
            if (cur && tskid == cur->id)
                _KERNEL_RET(E_CTX);
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
    if (tskid == TSK_SELF) {
        tcb = (T_TCB *)_kernel_cur;
    } else if (!(tcb = _kernel_tcb[tskid - 1])) {
        _KERNEL_END(E_NOEXS);
    }
    switch (tcb->sts) {
    case TTS_RDY:
        /*
         * make SUSPENDED
         */
        _kernel_deq((T_LNK *)tcb);
        tcb->sts = TTS_SUS;
        tcb->sus = 1;
        if (tcb == _kernel_cur)
            _kernel_highest();  /* dispatch */
        break;
    case TTS_WAI :
        /*
         * make WAITING-SUSPENDED
         */
        tcb->sts = TTS_WAS;
        tcb->sus = 1;
        break;
    case TTS_SUS :
    case TTS_WAS :
        if (tcb->sus == TMAX_SUSCNT)
            _KERNEL_END(E_QOVR);
        tcb->sus++;
        break;
    default : /* TTS_DMT */
        _KERNEL_END(E_OBJ);
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_SUS_TSK();
    return r;
}

/* end */
