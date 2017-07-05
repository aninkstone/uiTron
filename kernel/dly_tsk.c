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
 * dly_tsk system call
 */
ER dly_tsk(RELTIM tmout)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_ISLD(_kernel_sts))
        _KERNEL_RET(E_CTX);
#endif /* _KERNEL_NO_STATIC_ERR */

#ifdef _KERNEL_N4
    if (!tmout)
        goto ret;
#endif

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get TCB
     */
    tcb = (T_TCB *)_kernel_cur;
    /*
     * delay waiting
     */
    _kernel_deq((T_LNK *)tcb);
    tcb->sts = TTS_WAI;
    tcb->wai = TTW_DLY;
    {
        T_TIM *tim = &tcb->tim;
#ifdef _KERNEL_N4
        tim->tmo = _kernel_sysclk + tmout;
#else
        tim->tmo = _kernel_sysclk + tmout + 1;
#endif
        _kernel_enq_tmq(&_kernel_tmrq, tim);
    }
    _kernel_enq(&_kernel_dmy_que, (T_LNK *)tcb);
    _kernel_highest();          /* dispatch */
    /*
     * waken up
     */
    r = _KERNEL_GET_TCB_RET(tcb);
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DLY_TSK();
    return r;
}

/* eof */
