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
 * def_tex system call
 */
ER def_tex(ID tskid, const T_DTEX *pk_dtex)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;
    TEXPTN ptn;
    STAT stat;
    
#ifndef _KERNEL_NO_STATIC_ERR
    {
        UB s = _kernel_sts;
        /*
         * check context
         */
        if (_KERNEL_CHK_IL(s))
            _KERNEL_RET(E_CTX);
        /*
         * check par
         */
        if (tskid == TSK_SELF) {
            if (_KERNEL_CHK_SYS(s))
                _KERNEL_RET(E_ID);
        } else if (_kernel_tskid_max < (UINT)tskid) {
            _KERNEL_RET(E_ID);
        }
        if (pk_dtex && !pk_dtex->texrtn)
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get TCB
     */
    if (tskid == TSK_SELF) {
        tcb = (T_TCB *)_kernel_cur;
    } else if (!(tcb = _kernel_tcb[tskid - 1])) {
        _KERNEL_END(E_NOEXS);
    }
    /*
     * create tex
     */
    if (pk_dtex) {
        T_TEX *old = (T_TEX *)tcb->tex;
        T_TEX *new;
        /*
         * save old info and free tex
         */
        if (old) {
            ptn  = old->ptn;
            stat = old->stat;
            _kernel_sys_free((VP)old);
        }
        /*
         * alloc tex
         */
        if ((char *)pk_dtex < _KERNEL_ETEXT()) {
            if (!(new = (T_TEX *)_kernel_sys_alloc(sizeof(T_TEX))))
                _KERNEL_END(E_SYS);
            new->dtex = (T_DTEX *)pk_dtex;
        } else {
            if (!(new = (T_TEX *)_kernel_sys_alloc(sizeof(T_TEX) + sizeof(T_DTEX))))
                _KERNEL_END(E_SYS);
            new->dtex = (T_DTEX *)(new + 1);
            *new->dtex = *pk_dtex;
        }
        /*
         * resotre old info or init info
         */
        if (old) {
            new->ptn  = ptn;
            new->stat = stat;
        } else {
            new->ptn  = 0;
            new->stat = TTEX_DIS;
        }
        tcb->tex = new;
    }
    /*
     * remove tex
     */
    else {
        T_TEX *tex = (T_TEX *)tcb->tex;  /* avoid volatile effect */
        if (tex) {
            _kernel_sys_free((VP)tex);
            tcb->tex = NULL;
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DEF_TEX();
    return r;
}

/* end */
