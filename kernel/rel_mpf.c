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
 * rel_mpf system call
 */
ER rel_mpf(ID id, VP blf)
{
    UINT psw;
    ER r = E_OK;
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
    if (!blf)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * release memory block
     */
    if (!(mpf = _kernel_mpf[id - 1]))
        _KERNEL_END(E_NOEXS);
    if ((char *)blf < mpf->str || mpf->end <= (char *)blf)
        _KERNEL_END(E_PAR);
    {
        T_TCB *tcb;
        VP *p_blf;
        /*
         * wake up get-memory-block-waitng task
         */
        if (!mpf->cnt &&
            (tcb = (T_TCB *)_kernel_first(&mpf->que[0],
                                          (mpf->cmpf->mpfatr & TA_TPRI)?
                                          _kernel_tpri_max : 1)) != NULL) {
            p_blf = (VP)_KERNEL_GET_TCB_DAT(tcb);
            *p_blf = blf;
            _KERNEL_SET_TCB_RET(tcb, E_OK);
            _kernel_deq_rdy_dsp(tcb); /* dispatch */
        }
        /*
         * release only
         */
        else {
            mpf->cnt++;
            _kernel_enq_msg(&mpf->free, (T_MSG *)blf, 1);
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_REL_MPF();
    return r;
}

/* eof */
