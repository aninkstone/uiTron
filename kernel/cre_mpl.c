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
 * cre_mpl/acre_mpl system call
 */
ER cre_mpl(ID mplid, const T_CMPL *pk_cmpl)
{
    UINT psw;
    ER r = E_OK;
    ID id = mplid;
    T_MPL **tbl;
    
    /*
     * init hook
     */
    _kernel_chk_wmpl_fp = _kernel_chk_wmpl;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_mplid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cmpl || !pk_cmpl->mplsz ||
        (unsigned int)pk_cmpl->mpl & TMSK_ALIGN)
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
        id = _kernel_mplid_max;
        tbl = &_kernel_mpl[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_mpl[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init MPL
     */
    {
        /*
         * alloc MPL struct
         */
        T_MPL *mpl;
        SIZE siz = sizeof(T_MPL);
        PRI pri = 1;
        if (pk_cmpl->mplatr & TA_TPRI) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if ((char *)pk_cmpl < _KERNEL_ETEXT()) {
            if (!(mpl = (T_MPL *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            mpl->cmpl = (T_CMPL *)pk_cmpl;
        } else {
            if (!(mpl = (T_MPL *)_kernel_sys_alloc(siz + sizeof(T_CMPL))))
                _KERNEL_END(E_SYS);
            mpl->cmpl = (T_CMPL *)((char *)mpl + siz);
            *mpl->cmpl = *pk_cmpl;
        }
        /*
         * alloc memory pool
         */
        {
            char *blk;
            if (pk_cmpl->mpl) {
                blk = pk_cmpl->mpl;
            } else {
                if (!(blk = (char *)_kernel_mpl_alloc(pk_cmpl->mplsz))) {
                    _kernel_sys_free(mpl);
                    _KERNEL_END(E_NOMEM);
                }
            }
            _kernel_ini_mem(&mpl->head, blk, pk_cmpl->mplsz);
        }
        *tbl = mpl;
        /*
         * init
         */
        _kernel_nul_que(&mpl->que[0], pri);
    }
    /*
     * acre_??? will return id
     */
    if (mplid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_MPL();
    return r;
}

/* eof */
