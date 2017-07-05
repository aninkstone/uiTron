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
 * sig_sem/isig_sem system call
 */
ER sig_sem(ID id)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    T_SEM *sem;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_semid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * increment case
     */
    if (!(sem = _kernel_sem[id - 1]))
        _KERNEL_END(E_NOEXS);
    if (sem->cnt ||
        !(tcb = (T_TCB *)_kernel_first(&sem->que[0],
                                       (sem->csem->sematr & TA_TPRI)?
                                       _kernel_tpri_max : 1))) {
        if (sem->cnt != sem->csem->maxsem) {
            sem->cnt++;
        } else {
            _KERNEL_END(E_QOVR);
        }
    }
    /*
     * wake up case
     */
    else {
        _KERNEL_SET_TCB_RET(tcb, E_OK);
        _kernel_deq_rdy_dsp(tcb); /* dispatch */
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_SIG_SEM();
    return r;
}

/* eof */
