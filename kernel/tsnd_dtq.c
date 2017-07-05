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
 * snd_dtq/psnd_dtq/ipsnd_dtq/tsnd_dtq system call
 */
ER tsnd_dtq(ID id, VP_INT data, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_DTQ *dtq;

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
        if (!id || _kernel_dtqid_max < (UINT)id)
            _KERNEL_RET(E_ID);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * send data
     */
    if (!(dtq = _kernel_dtq[id - 1]))
        _KERNEL_END(E_NOEXS);
    {
        UINT cnt;
        UINT dtqcnt;
        /*
         * no bufferd data and receive task exist : wake up
         */
        cnt = dtq->cnt;
        if (!cnt) {
            T_TCB *tcb = (T_TCB *)dtq->rque.n;
            if (tcb != (T_TCB *)&dtq->rque) {
                _KERNEL_SET_TCB_DAT(tcb, data);
                _KERNEL_SET_TCB_RET(tcb, E_OK);
                _kernel_deq_rdy_dsp(tcb); /* dispatch */
                goto end;
            }
        }
        /*
         * store data to buffer
         */
        dtqcnt = dtq->cdtq->dtqcnt;
        if (cnt != dtqcnt) {
            UINT ip;
            dtq->cnt = cnt + 1;
            ip = dtq->ip;
            if (dtq->cdtq->dtqatr & TA_BYTE) { /* byte queue */
                ((UB *)dtq->buf)[ip] = (UB)(int)data;
            } else {                           /* word queue */
                dtq->buf[ip] = data;
            }
            if (++ip == dtqcnt)
                ip = 0;
            dtq->ip = ip;
        }
        /*
         * buffer full : send data queue waiting
         */
        else {
            if (tmout == TMO_POL) {
                r = E_TMOUT;
            } else {
                T_TCB *tcb = (T_TCB *)_kernel_cur;
                tcb->wai = TTW_SDTQ;
                tcb->wid = id;
                _KERNEL_SET_REGPAR_DAT(data);
                _KERNEL_SET_REGPAR_FLG();
                r = _kernel_wai(tcb, tmout, &dtq->sque[0], 
                                dtq->cdtq->dtqatr & TA_TPRI); /* dispatch */
            }
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TSND_DTQ();
    return r;
}

/* eof */
