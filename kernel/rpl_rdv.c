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
 * rpl_por system call
 */
ER rpl_rdv(RDVNO rdvno, VP msg, UINT rmsgsz)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!msg && rmsgsz)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get TCB
     */
    {
        ID tid = rdvno & 0xff;
        if (!tid                          ||
            _kernel_tskid_max < (UINT)tid ||
            !(tcb = _kernel_tcb[tid - 1]) ||
            !(tcb->sts & TTS_WAI)         ||
            tcb->wai != TTW_RDV            )
            _KERNEL_END(E_OBJ);
    }
    /*
     * check rendezvous number
     */
    {
        RDVNO rdv = _KERNEL_GET_TCB_RDV(tcb);
        if (rdv != rdvno)
            _KERNEL_END(E_OBJ);
    }
    /*
     * check msg size
     */
    {
        UINT maxrmsg = _KERNEL_GET_TCB_SIZ(tcb);
        if (maxrmsg < rmsgsz)
            _KERNEL_END(E_PAR);
    }
    /*
     * copy data
     */
    if (rmsgsz) {
        UB *dat = _KERNEL_GET_TCB_DAT(tcb);
        memcpy(dat, msg, rmsgsz);
    }
    /*
     * wake up reply-rendezvous-waiting task
     */
    _KERNEL_SET_TCB_RET(tcb, rmsgsz);
    _kernel_deq_rdy_dsp(tcb);   /* dispatch */
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_RPL_RDV();
    return r;
}

/* eof */
