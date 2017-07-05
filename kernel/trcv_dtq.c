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
 * rcv_dtq/prcv_dtq/trcv_dtq system call
 */
ER trcv_dtq(ID id, VP_INT *p_data, TMO tmout)
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
        if (!p_data)
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * receive data
     */
    if (!(dtq = _kernel_dtq[id - 1]))
        _KERNEL_END(E_NOEXS);
    {
        UINT cnt = dtq->cnt;
        UINT dtqcnt = dtq->cdtq->dtqcnt;
        /*
         * buffer full or no buffer case
         */
        if (cnt == dtqcnt) {
            /*
             * send waiting task exist
             */
            T_TCB *tcb =
                (T_TCB *)_kernel_first(&dtq->sque[0],
                                       (dtq->cdtq->dtqatr & TA_TPRI)?
                                       _kernel_tpri_max : 1);
            if (tcb) {
                /*
                 * no buffer DTQ : get data directly
                 */
                VP_INT data = _KERNEL_GET_TCB_DAT(tcb);
                if (!cnt) {
                    if (dtq->cdtq->dtqatr & TA_BYTE) /* byte queue */
                        data = (VP_INT)((int)data & 0xff);
                    *p_data = data;
                }
                /*
                 * get data through buffer
                 */
                else {
                    UINT op = dtq->op;
                    if (dtq->cdtq->dtqatr & TA_BYTE) { /* byte queue */
                        *p_data = (VP_INT)(int)(((UB *)dtq->buf)[op]);
                        ((UB *)dtq->buf)[op] = (UB)(int)data; /* buffer full : ip == op */
                    } else {                           /* word queue */
                        *p_data = dtq->buf[op];
                        dtq->buf[op] = data;                  /* buffer full : ip == op */
                    }
                    if (++op == dtqcnt)
                        op = 0;
                    dtq->ip = dtq->op = op;
                }
                /*
                 * wake up
                 */
                _KERNEL_SET_TCB_RET(tcb, E_OK);
                _kernel_deq_rdy_dsp(tcb); /* dispatch */
                goto end;
            }
        }
        /*
         * buffered data exist : get data from buffer
         */
        if (cnt) {
            UINT op = dtq->op;
            if (dtq->cdtq->dtqatr & TA_BYTE) { /* byte queue */
                *p_data = (VP_INT)(int)(((UB *)dtq->buf)[op]);
            } else {                           /* word queue */
                *p_data = dtq->buf[op];
            }
            if (++op == dtqcnt)
                op = 0;
            dtq->op = op;
            dtq->cnt = cnt - 1;
        }
        /*
         * receive data queue waiting
         */
        else {
            if (tmout == TMO_POL) {
                r = E_TMOUT;
            } else {
                T_TCB *tcb = (T_TCB *)_kernel_cur;
                tcb->wai = TTW_RDTQ;
                tcb->wid = id;
                r = _kernel_wai(tcb, tmout, &dtq->rque, 0); /* dispatch */
                if (r == E_OK)
                    *p_data = _KERNEL_GET_TCB_DAT(tcb);
            }
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TRCV_DTQ();
    return r;
}

/* eof */
