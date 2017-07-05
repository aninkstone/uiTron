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
#ifdef _KERNEL_MTX
#include "kernel.h"

/*
 * cre_mtx/acre_mtx system call
 */
ER cre_mtx(ID mtxid, const T_CMTX *pk_cmtx)
{
    UINT psw;
    ER r = E_OK;
    ID id = mtxid;
    T_MTX **tbl;
    
    /*
     * init hook
     */
    _kernel_chg_mtx_fp = _kernel_chg_mtx;
    _kernel_unl_amtx_fp = _kernel_unl_amtx;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_mtxid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cmtx || (UINT)_kernel_tpri_max < (UINT)pk_cmtx->ceilpri)
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
        id = _kernel_mtxid_max;
        tbl = &_kernel_mtx[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_mtx[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init MTX
     */
    {
        /*
         * alloc
         */
        T_MTX *mtx;
        SIZE siz = sizeof(T_MTX);
        PRI pri = 1;
        if (pk_cmtx->mtxatr != TA_TFIFO) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if ((char *)pk_cmtx < _KERNEL_ETEXT()) {
            if (!(mtx = (T_MTX *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            mtx->cmtx = (T_CMTX *)pk_cmtx;
        } else {
            if (!(mtx = (T_MTX *)_kernel_sys_alloc(siz + sizeof(T_CMTX))))
                _KERNEL_END(E_SYS);
            mtx->cmtx = (T_CMTX *)((char *)mtx + siz);
            *mtx->cmtx = *pk_cmtx;
        }
        *tbl = mtx;
        /*
         * init
         */
        mtx->loc = NULL;
        _kernel_nul_que(&mtx->que[0], pri);
    }
    /*
     * acre_??? will return id
     */
    if (mtxid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_MTX();
    return r;
}

#endif /* _KERNEL_MTX */
/* eof */
