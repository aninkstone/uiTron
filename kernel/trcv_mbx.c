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
 * rcv_mbx/prcv_mbx/trcv_mbx system call
 */
ER trcv_mbx(ID id, T_MSG **ppk_msg, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    
#ifndef _KERNEL_NO_STATIC_ERR
    {
        UB s = _kernel_sts;
        /*
         * check context
         */
        if (tmout == TMO_POL) {
            if (_KERNEL_CHK_LOC(s))
                _KERNEL_RET(E_CTX);
        } else {
            if (_KERNEL_CHK_WAI(s, tmout))
                _KERNEL_RET(E_CTX);
        }
        /*
         * check par
         */
        if (!id || _kernel_mbxid_max < (UINT)id)
            _KERNEL_RET(E_ID);
        if (!ppk_msg)
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * receive msg
     */
    {
        T_MBX *mbx;
        T_MSG *msg;

        if (!(mbx = _kernel_mbx[id - 1]))
            _KERNEL_END(E_NOEXS);
        msg = _kernel_deq_msg(mbx->mque,
                              (mbx->cmbx->mbxatr & TA_MPRI)? mbx->cmbx->maxmpri : 1);
        /*
         * msg exist case
         */
        if (msg) {
            *ppk_msg = msg;
        }
        /*
         * mailbox waiting case
         */
        else {
            if (tmout == TMO_POL) {
                r = E_TMOUT;
            } else {
                T_TCB *tcb = (T_TCB *)_kernel_cur;
                tcb->wai = TTW_MBX;
                tcb->wid = id;
                r = _kernel_wai(tcb, tmout, &mbx->que[0],
                                mbx->cmbx->mbxatr & TA_TPRI); /* dispatch */
                if (r == E_OK)
                    *ppk_msg = _KERNEL_GET_TCB_DAT(tcb);
            }
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TRCV_MBX();
    return r;
}

/* eof */
