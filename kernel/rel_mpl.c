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
 * wakeup get MPL waiting task
 *   return value :
 *     1 : require dispatch
 *     0 : not require
 */
B _kernel_chk_wmpl(T_MPL *mpl)
{
    B dsp = 0;
    PRI pri;
    T_LNK *que;
    
    /*
     * get memory block for get-MPL-waitng tasks
     */
    pri = (mpl->cmpl->mplatr & TA_TPRI)? _kernel_tpri_max : 1;
    que = &mpl->que[0];
    for (; pri--; que++) {
        T_TCB *tcb = (T_TCB *)que->n;
        while (tcb != (T_TCB *)que) {
            T_TCB *tcb_n = tcb->n;
            VP *p_blk   = _KERNEL_GET_TCB_DAT(tcb);
            SIZE blksiz = _KERNEL_GET_TCB_SIZ(tcb);
            VP buf = (VP)_kernel_alloc(&mpl->head, blksiz);
            if (!buf)
                goto end;       /* no more space */
            *p_blk = buf;
            /*
             * wake up waiting task
             */
            _KERNEL_SET_TCB_RET(tcb, E_OK);
            dsp |= _kernel_deq_rdy(tcb);
            tcb = tcb_n;
        }
    }
end:
    return dsp;
}

/*
 * rel_mpl system call
 */
ER rel_mpl(ID id, VP blk)
{
    UINT psw;
    ER r = E_OK;
    T_MPL *mpl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mplid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * release memory block
     */
    if (!(mpl = _kernel_mpl[id - 1]))
        _KERNEL_END(E_NOEXS);
    if ((r = _kernel_free(&mpl->head, blk)) != E_OK)
        _KERNEL_END(r);         /* E_PAR */
    /*
     * check get-MPL-waitng tasks
     */
    if (_kernel_chk_wmpl(mpl))
        _kernel_highest();      /* dispatch */
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_REL_MPL();
    return r;
}

/* eof */
