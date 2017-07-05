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
 * ter_tsk system call
 */
ER ter_tsk(ID tskid)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    B dsp = 0;
    
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
            _KERNEL_RET(E_ILUSE);
        } else if (_kernel_tskid_max < (UINT)tskid) {
            _KERNEL_RET(E_ID);
        } else if (_KERNEL_CHK_TSK(s) && tskid == _kernel_cur->id) {
            _KERNEL_RET(E_ILUSE);
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
     * remove from ready queue or waiting queue
     */
    switch (tcb->sts) {
    case TTS_RDY:
        _kernel_deq((T_LNK *)tcb);
        goto next;
    case TTS_WAI:
    case TTS_WAS:
        if (tcb->tim.n)
            _kernel_deq((T_LNK *)&tcb->tim); /* remove from timer queue */
        _kernel_deq((T_LNK *)tcb);
        if (tcb->wai == TTW_SMBF) {
            dsp = (*_kernel_chk_wmbf_fp)(_kernel_mbf[tcb->wid - 1]);
        } else if (tcb->wai == TTW_MPL) {
            dsp = (*_kernel_chk_wmpl_fp)(_kernel_mpl[tcb->wid - 1]);
        }
        /* goto next; */
    case TTS_SUS:
    next:
        /*
         * stop overrun handler
         */
        if (_kernel_ovrcnt)
            _kernel_ovrcnt[tcb->id - 1] = 0;

#ifdef _KERNEL_MTX
        /*
         * unlock all mutexes
         */
        (*_kernel_unl_amtx_fp)(tcb);
#endif /* _KERNEL_MTX */

        /*
         * restart if act queueing
         */
        if (tcb->act) {
            tcb->act--;
            _kernel_act(tcb, tcb->ctsk->exinf); /* dispatch */
        }
        /*
         * make DORMANT state
         */
        else {
            tcb->sts = TTS_DMT;
            if (tcb == _kernel_cur) {
                _kernel_cur = NULL; /* purge context */
                dsp = 1;
            }
            if (dsp)
                _kernel_highest(); /* dispatch */
        }
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
    _KERNEL_ASSERT_TER_TSK();
    return r;
}

/* end */
