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
 * del_mpf system call
 */
ER del_mpf(ID id)
{
    UINT psw;
    ER r = E_OK;
    T_MPF *mpf;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mpfid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * remove waiting tasks
     */
    if (!(mpf = _kernel_mpf[id - 1]))
        _KERNEL_END(E_NOEXS);
    _kernel_dlt(&mpf->que[0], 
                (mpf->cmpf->mpfatr & TA_TPRI)? _kernel_tpri_max : 1);
    /*
     * delete MPF
     */
    if (!mpf->cmpf->mpf)
        _kernel_mpl_free(mpf->str);
    _kernel_sys_free(mpf);
    _kernel_mpf[id - 1] = NULL;
    _kernel_highest();          /* dispatch */
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DEL_MPF();
    return r;
}

/* eof */
