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
 * cre_cyc/acre_cyc system call
 */
ER cre_cyc(ID cycid, const T_CCYC *pk_ccyc)
{
    UINT psw;
    ER r = E_OK;
    ID id = cycid;
    T_CYC **tbl;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY && (!id || _kernel_cycno_max < (UINT)id))
        _KERNEL_RET(E_ID);
    if (!pk_ccyc || !pk_ccyc->cychdr || !pk_ccyc->cyctim)
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
        id = _kernel_cycno_max;
        tbl = &_kernel_cyc[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_cyc[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    {
        /*
         * alloc
         */
        T_CYC *cyc;
        SIZE siz = sizeof(T_CYC);
        if ((char *)pk_ccyc < _KERNEL_ETEXT()) {
            if (!(cyc = (T_CYC *)_kernel_sys_alloc(siz)))
                _KERNEL_END(E_SYS);
            cyc->ccyc = (T_CCYC *)pk_ccyc;
        } else {
            if (!(cyc = (T_CYC *)_kernel_sys_alloc(siz + sizeof(T_CCYC))))
                _KERNEL_END(E_SYS);
            cyc->ccyc = (T_CCYC *)((char *)cyc + siz);
            *cyc->ccyc = *pk_ccyc;
        }
        *tbl = cyc;
        /*
         * init
         */
        cyc->sta = (pk_ccyc->cycatr & TA_STA)? 1 : 0;
        /*
         * start cyc timer
         */
        if (pk_ccyc->cycatr & (TA_STA | TA_PHS)) {
#ifdef _KERNEL_N4
            ((T_TIM *)cyc)->tmo = pk_ccyc->cycphs + _kernel_sysclk;
#else
            ((T_TIM *)cyc)->tmo = pk_ccyc->cycphs + _kernel_sysclk + 1;
#endif
            _kernel_enq_tmq(&_kernel_cycq, (T_TIM *)cyc);
        }
    }
    /*
     * acre_??? will return id
     */
    if (cycid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_CYC();
    return r;
}

/* eof */
