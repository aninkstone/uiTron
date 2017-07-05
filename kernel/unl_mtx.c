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
#ifdef _KERNEL_MTX
#include "kernel.h"

/*
 * unlock all locked mutex of the task
 */
B _kernel_unl_amtx(T_TCB *atcb)
{
    B dsp = 0;
    T_MTX **mtxp = &_kernel_mtx[0];
    UINT i = _kernel_mtxid_max;

    for (; i--; mtxp++) {
        if (*mtxp && (*mtxp)->loc == atcb) {
            T_MTX *mtx = *mtxp;
            T_TCB *tcb;
            /*
             * no waiting task : unlock only
             */
            if (!(tcb = (T_TCB *)_kernel_first(&mtx->que[0],
                                               (mtx->cmtx->mtxatr != TA_TFIFO)?
                                               _kernel_tpri_max : 1))) {
                mtx->loc = NULL; /* unlock */
            }
            /*
             * lock by next waiting task
             */
            else {
                mtx->loc = tcb; /* update lock task */
                /*
                 * TA_CEILING
                 */
                if (mtx->cmtx->mtxatr == TA_CEILING) {
                    if (mtx->cmtx->ceilpri < tcb->pri)
                        tcb->pri = mtx->cmtx->ceilpri;
                }
                /*
                 * TA_INHERIT
                 */
                else if (mtx->cmtx->mtxatr == TA_INHERIT) {
                    tcb->pri = atcb->pri;
                }
                /*
                 * wake up
                 */
                _KERNEL_SET_TCB_RET(tcb, E_OK);
                dsp |= _kernel_deq_rdy(tcb);
            }
        }
    }
    return dsp;
}

/*
 * unl_mtx system call
 */
ER unl_mtx(ID id)
{
    UINT psw;
    ER r = E_OK;
    T_MTX *mtx;
    T_TCB *tcb;
    UB opri;
    B dsp = 0;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_TSK(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mtxid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * decrement lock counter and revert base priority
     */
    if (!(mtx = _kernel_mtx[id - 1]))
        _KERNEL_END(E_NOEXS);
    if (mtx->loc != _kernel_cur)
        _KERNEL_END(E_ILUSE);
    tcb = (T_TCB *)_kernel_cur;
    opri = tcb->pri;
    /*
     * all unlocked : revert to base priority
     */
    if (!--tcb->mtx && tcb->pri < tcb->bpri) {
        tcb->pri = tcb->bpri;
        _kernel_deq((T_LNK *)tcb);
        _kernel_enq(&_kernel_rdq[tcb->pri - 1], (T_LNK *)tcb);
        dsp = 1;
    }
    /*
     * no waiting task : unlock only
     */
    if (!(tcb = (T_TCB *)_kernel_first(&mtx->que[0],
                                       (mtx->cmtx->mtxatr != TA_TFIFO)?
                                       _kernel_tpri_max : 1))) {
        mtx->loc = NULL;        /* unlock */
    }
    /*
     * lock by next waiting task
     */
    else {
        mtx->loc = tcb;         /* update lock task */
        /*
         * TA_CEILING
         */
        if (mtx->cmtx->mtxatr == TA_CEILING) {
            if (mtx->cmtx->ceilpri < tcb->pri)
                tcb->pri = mtx->cmtx->ceilpri;
        }
        /*
         * TA_INHERIT
         */
        else if (mtx->cmtx->mtxatr == TA_INHERIT) {
            tcb->pri = opri;
        }
        /*
         * wake up
         */
        _KERNEL_SET_TCB_RET(tcb, E_OK);
        dsp |= _kernel_deq_rdy(tcb);
    }
    if (dsp)
        _kernel_highest();      /* dispatch */
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_UNL_MTX();
    return r;
}

#endif /* _KERNEL_MTX */
/* eof */
