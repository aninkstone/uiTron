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

ER rot_rdq(PRI tskpri)
{
    UINT psw;
    ER r = E_OK;
    
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
        if ((tskpri == TPRI_SELF && _KERNEL_CHK_IS(s))
            || (UINT)_kernel_tpri_max < (UINT)tskpri)
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get ready queue
     */
    {
        T_LNK *que;
        T_TCB *tcb;
        if (tskpri == TPRI_SELF)
#ifdef _KERNEL_MTX
            tskpri = _kernel_cur->bpri;
#else /* _KERNEL_MTX */
            tskpri = _kernel_cur->pri;
#endif /* _KERNEL_MTX */
        que = &_kernel_rdq[tskpri - 1];
        /*
         * the queue has 2 or more task : rotate
         */
        tcb = (T_TCB *)que->n;
        if (tcb != (T_TCB *)que && tcb->n != (T_TCB *)que) {
            _kernel_deq((T_LNK *)tcb);
            _kernel_enq(que, (T_LNK *)tcb);
            if (tcb == _kernel_cur)
                _kernel_run((T_TCB *)que->n);
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_ROT_RDQ();
    return r;
}

/* end */
