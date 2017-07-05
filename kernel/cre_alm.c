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
 * cre_alm/acre_alm system call
 */
ER cre_alm(ID almid, const T_CALM *pk_calm)
{
    UINT psw;
    ER r = E_OK;
    ID id = almid;
    T_ALM **tbl;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_almno_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_calm || !pk_calm->almhdr)
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
        id = _kernel_almno_max;
        tbl = &_kernel_alm[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_alm[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    {
        /*
         * alloc
         */
        T_ALM *alm;
        SIZE siz = sizeof(T_ALM);
        if ((char *)pk_calm < _KERNEL_ETEXT()) {
            if (!(alm = (T_ALM *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            alm->calm = (T_CALM *)pk_calm;
        } else {
            if (!(alm = (T_ALM *)_kernel_sys_alloc(siz + sizeof(T_CALM))))
                _KERNEL_END(E_SYS);
            alm->calm = (T_CALM *)((char *)alm + siz);
            *alm->calm = *pk_calm;
        }
        *tbl = alm;
    }
    /*
     * acre_??? will return id
     */
    if (almid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_ALM();
    return r;
}

/* eof */
