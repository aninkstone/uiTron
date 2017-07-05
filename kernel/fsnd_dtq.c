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
 * fsnd_dtq/ifsnd_dtq system call
 */
ER fsnd_dtq(ID id, VP_INT data)
{
    UINT psw;
    ER r = E_OK;
    T_DTQ *dtq;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_dtqid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * force send data
     */
    if (!(dtq = _kernel_dtq[id - 1]))
        _KERNEL_END(E_NOEXS);
    {
        UINT cnt;
        UINT dtqcnt;
        /*
         * check buffer size
         */
        if (!(dtqcnt = dtq->cdtq->dtqcnt))
            _KERNEL_END(E_ILUSE);
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
        {
            /*
             * force store data to buffer
             */
            UINT ip = dtq->ip;
            if (dtq->cdtq->dtqatr & TA_BYTE) { /* byte queue */
                ((UB *)dtq->buf)[ip] = (UB)(int)data;
            } else {                           /* word queue */
                dtq->buf[ip] = data;
            }
            if (++ip == dtqcnt)
                ip = 0;
            dtq->ip = ip;
            if (cnt != dtqcnt) {
                dtq->cnt = cnt + 1;
            }
            /*
             * buffer full : delete first data
             */
            else {
                dtq->op = ip;
            }
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_FSND_DTQ();
    return r;
}

/* eof */
