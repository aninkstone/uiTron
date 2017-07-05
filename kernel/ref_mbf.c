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
 * ref_mbf system call
 */
ER ref_mbf(ID id, T_RMBF *pk_rmbf)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    T_MBF *mbf;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mbfid_max < (UINT)id)
        _KERNEL_RET(E_ID);
    if (!pk_rmbf)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get info
     */
    if (!(mbf = _kernel_mbf[id - 1]))
        _KERNEL_END(E_NOEXS);
    {
        PRI spri = (mbf->cmbf->mbfatr & TA_TPRI )? _kernel_tpri_max : 1;
        PRI rpri = (mbf->cmbf->mbfatr & TA_TPRIR)? _kernel_tpri_max : 1;
        tcb = (T_TCB *)_kernel_first(&mbf->sque[0], spri);
        pk_rmbf->stskid = (tcb)? tcb->id : TSK_NONE;
        tcb = (T_TCB *)_kernel_first(&mbf->sque[spri], rpri);
        /*                           &mbf->rque[0] */
        pk_rmbf->rtskid = (tcb)? tcb->id : TSK_NONE;
        pk_rmbf->smsgcnt = mbf->smsgcnt;
        pk_rmbf->fmbfsz = mbf->cmbf->mbfsz - mbf->sz;
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_REF_MBF();
    return r;
}

/* eof */
