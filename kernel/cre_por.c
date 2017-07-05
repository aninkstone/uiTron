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
 * cre_por/acre_por system call
 */
ER cre_por(ID porid, const T_CPOR *pk_cpor)
{
    UINT psw;
    ER r = E_OK;
    ID id = porid;
    T_POR **tbl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_porid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_cpor)
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
        id = _kernel_porid_max;
        tbl = &_kernel_por[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_por[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init POR
     */
    {
        /*
         * alloc POR struct and ring buffer
         */
        T_POR *por;
        SIZE siz = sizeof(T_POR);
        PRI pri = 1;
        if (pk_cpor->poratr & TA_TPRI) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if ((char *)pk_cpor < _KERNEL_ETEXT()) {
            if (!(por = (T_POR *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            por->cpor = (T_CPOR *)pk_cpor;
        } else {
            if (!(por = (T_POR *)_kernel_sys_alloc(siz + sizeof(T_CPOR))))
                _KERNEL_END(E_SYS);
            por->cpor = (T_CPOR *)((char *)por + siz);
            *por->cpor = *pk_cpor;
        }
        *tbl = por;
        /*
         * init
         */
        _kernel_nul_que(&por->aque, 1);
        _kernel_nul_que(&por->cque[0], pri);
    }
    /*
     * acre_??? will return id
     */
    if (porid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_POR();
    return r;
}

/* eof */
