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
 * slp_tsk/tslp_tsk system call
 */
ER tslp_tsk(TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    TMO tmo;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_WAI(_kernel_sts, tmout))
        _KERNEL_RET(E_CTX);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get TCB
     */
    tcb = (T_TCB *)_kernel_cur;
    /*
     * queueing count down
     */
    if (tcb->wup) {
        tcb->wup--;
    }
    /* 
     * sleep waiting
     */
    else {
        if (tmout == TMO_POL) {
            r = E_TMOUT;
        } else {
            tcb->wai = TTW_SLP;
            r = _kernel_wai(tcb, tmout, &_kernel_dmy_que, 0); /* dispatch */
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TSLP_TSK();
    return r;
}

/* end */
