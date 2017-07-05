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
 * del_mbf system call
 */
ER del_mbf(ID id)
{
    UINT psw;
    ER r = E_OK;
    T_MBF *mbf;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mbfid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * remove waiting tasks
     */
    if (!(mbf = _kernel_mbf[id - 1]))
        _KERNEL_END(E_NOEXS);
    {
        PRI spri = (mbf->cmbf->mbfatr & TA_TPRI )? _kernel_tpri_max : 1;
        PRI rpri = (mbf->cmbf->mbfatr & TA_TPRIR)? _kernel_tpri_max : 1;
        _kernel_dlt(&mbf->sque[0], spri);
        _kernel_dlt(&mbf->sque[spri], rpri);
        /*          &mbf->rque[0] */
    }
    /*
     * delete MBF
     */
    if (mbf->cmbf->mbfsz && !mbf->cmbf->mbf)
        _kernel_mpl_free(mbf->buf);
    _kernel_sys_free(mbf);
    _kernel_mbf[id - 1] = NULL;
    _kernel_highest();          /* dispatch */
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DEL_MBF();
    return r;
}

/* eof */
