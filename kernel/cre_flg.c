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
 * cre_flg/acre_flg system call
 */
ER cre_flg(ID flgid, const T_CFLG *pk_cflg)
{
    UINT psw;
    ER r = E_OK;
    ID id = flgid;
    T_FLG **tbl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_flgid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cflg)
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
        id = _kernel_flgid_max;
        tbl = &_kernel_flg[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_flg[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init FLG
     */
    {
        /*
         * alloc
         */
        T_FLG *flg;
        SIZE siz = sizeof(T_FLG);
        PRI pri = 1;
        if (!(~pk_cflg->flgatr & (TA_WMUL | TA_TPRI))) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if ((char *)pk_cflg < _KERNEL_ETEXT()) {
            if (!(flg = (T_FLG *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            flg->cflg = (T_CFLG *)pk_cflg;
        } else {
            if (!(flg = (T_FLG *)_kernel_sys_alloc(siz + sizeof(T_CFLG))))
                _KERNEL_END(E_SYS);
            flg->cflg = (T_CFLG *)((char *)flg + siz);
            *flg->cflg = *pk_cflg;
        }
        *tbl = flg;
        /*
         * init
         */
        flg->flgptn = pk_cflg->iflgptn;
        _kernel_nul_que(&flg->que[0], pri);
    }
    /*
     * acre_??? will return id
     */
    if (flgid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_FLG();
    return r;
}

/* eof */
