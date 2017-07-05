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
 * cre_mbx/acre_mbx system call
 */
ER cre_mbx(ID mbxid, const T_CMBX *pk_cmbx)
{
    UINT psw;
    ER r = E_OK;
    ID id = mbxid;
    T_MBX **tbl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_mbxid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cmbx || !pk_cmbx->maxmpri)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * alloc auto id
     */
    if (id == ID_ANY) {
        id = _kernel_mbxid_max;
        tbl = &_kernel_mbx[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_mbx[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init MBX
     */
    {
        /*
         * alloc MBX struct
         */
        T_MBX *mbx;
        SIZE siz = sizeof(T_MBX);
        PRI pri = 1;
        PRI maxmpri = 1;
        T_LNK *que;
        if (pk_cmbx->mbxatr & TA_TPRI) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if (pk_cmbx->mbxatr & TA_MPRI)
            maxmpri = pk_cmbx->maxmpri;
        if (!pk_cmbx->mprihd)
            siz += TSZ_MPRIHD(maxmpri);
        if ((char *)pk_cmbx < _KERNEL_ETEXT()) {
            if (!(mbx = (T_MBX *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            mbx->cmbx = (T_CMBX *)pk_cmbx;
        } else {
            if (!(mbx = (T_MBX *)_kernel_sys_alloc(siz + sizeof(T_CMBX))))
                _KERNEL_END(E_SYS);
            mbx->cmbx = (T_CMBX *)((char *)mbx + siz);
            *mbx->cmbx = *pk_cmbx;
        }
        mbx->mque = (pk_cmbx->mprihd)?
            (T_MSGQUE *)pk_cmbx->mprihd : (T_MSGQUE *)&mbx->que[pri];
        *tbl = mbx;
        /*
         * init
         */
        _kernel_nul_que(&mbx->que[0], pri);
        _kernel_nul_msg_que(mbx->mque, maxmpri);
    }
    /*
     * acre_??? will return id
     */
    if (mbxid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_MBX();
    return r;
}

/* eof */
