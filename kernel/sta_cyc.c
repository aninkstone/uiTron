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
 * sta_cyc system call
 */
ER sta_cyc(ID id)
{
    UINT psw;
    ER r = E_OK;
    T_CYC *cyc;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_cycno_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * enable cyc handler
     */
    if (!(cyc = _kernel_cyc[id - 1]))
        _KERNEL_END(E_NOEXS);
    cyc->sta = 1;
    /*
     * not TA_PHS : restart cyc timer
     */
    if (!(cyc->ccyc->cycatr & TA_PHS)) {
#ifdef _KERNEL_N4
        ((T_TIM *)cyc)->tmo = cyc->ccyc->cyctim + _kernel_sysclk;
#else
        ((T_TIM *)cyc)->tmo = cyc->ccyc->cyctim + _kernel_sysclk + 1;
#endif
        if (((T_TIM *)cyc)->n)
            _kernel_deq_tmq(&_kernel_cycq, (T_TIM *)cyc);
        _kernel_enq_tmq(&_kernel_cycq, (T_TIM *)cyc);
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_STA_CYC();
    return r;
}

/* eof */
