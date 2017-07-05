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
 * snd_mbx system call
 */
ER snd_mbx(ID id, T_MSG *pk_msg)
{
    UINT psw;
    ER r = E_OK;
    T_MBX *mbx;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mbxid_max < (UINT)id)
        _KERNEL_RET(E_ID);
    if (!pk_msg)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * send msg
     */
    {
        T_MBX *mbx;
        T_TCB *tcb;

        if (!(mbx = _kernel_mbx[id - 1]))
            _KERNEL_END(E_NOEXS);
        /*
         * queueing case
         */
        if (!(tcb = (T_TCB *)_kernel_first(&mbx->que[0],
                                           (mbx->cmbx->mbxatr & TA_TPRI)?
                                           _kernel_tpri_max : 1))) {
            PRI msgpri = 1;
            if (mbx->cmbx->mbxatr & TA_MPRI) {
                msgpri = ((T_MSG_PRI *)pk_msg)->msgpri;
                if (!msgpri || mbx->cmbx->maxmpri < msgpri)
                    _KERNEL_END(E_PAR);
            }
            _kernel_enq_msg(mbx->mque, pk_msg, msgpri);
        }
        /*
         * wake up case
         */
        else {
            _KERNEL_SET_TCB_DAT(tcb, pk_msg);
            _KERNEL_SET_TCB_RET(tcb, E_OK);
            _kernel_deq_rdy_dsp(tcb); /* dispatch */
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_SND_MBX();
    return r;
}

/* eof */
