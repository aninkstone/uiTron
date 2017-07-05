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
 * ref_mpf system call
 */
ER ref_mpf(ID id, T_RMPF *pk_rmpf)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    T_MPF *mpf;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mpfid_max < (UINT)id)
        _KERNEL_RET(E_ID);
    if (!pk_rmpf)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get info
     */
    if (!(mpf = _kernel_mpf[id - 1]))
        _KERNEL_END(E_NOEXS);
    tcb = (T_TCB *)_kernel_first(&mpf->que[0],
                                 (mpf->cmpf->mpfatr & TA_TPRI)?
                                 _kernel_tpri_max : 1);
    pk_rmpf->wtskid = (tcb)? tcb->id : TSK_NONE;
    pk_rmpf->fblkcnt = mpf->cnt;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_REF_MPF();
    return r;
}

/* eof */
