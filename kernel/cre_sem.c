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
 * cre_sem/acre_sem system call
 */
ER cre_sem(ID semid, const T_CSEM *pk_csem)
{
    UINT psw;
    ER r = E_OK;
    ID id = semid;
    T_SEM **tbl;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_semid_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_csem ||
#if 2 < _KERNEL_INTSIZ
        TMAX_MAXSEM < pk_csem->maxsem ||
#endif
        pk_csem->maxsem < pk_csem->isemcnt)
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
        id = _kernel_semid_max;
        tbl = &_kernel_sem[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_sem[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * init SEM
     */
    {
        /*
         * alloc
         */
        T_SEM *sem;
        SIZE siz = sizeof(T_SEM);
        PRI pri = 1;
        if (pk_csem->sematr & TA_TPRI) {
            pri = _kernel_tpri_max;
            siz += sizeof(T_LNK) * (pri - 1);
        }
        if ((char *)pk_csem < _KERNEL_ETEXT()) {
            if (!(sem = (T_SEM *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            sem->csem = (T_CSEM *)pk_csem;
        } else {
            if (!(sem = (T_SEM *)_kernel_sys_alloc(siz + sizeof(T_CSEM))))
                _KERNEL_END(E_SYS);
            sem->csem = (T_CSEM *)((char *)sem + siz);
            *sem->csem = *pk_csem;
        }
        *tbl = sem;
        /*
         * init
         */
        sem->cnt = pk_csem->isemcnt;
        _kernel_nul_que(&sem->que[0], pri);
    }
    /*
     * acre_??? will return id
     */
    if (semid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_SEM();
    return r;
}

/* eof */
