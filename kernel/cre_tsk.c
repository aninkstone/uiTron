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
 * cre_tsk/acre_tsk system call
 */
ER cre_tsk(ID tskid, const T_CTSK *pk_ctsk)
{
    UINT psw;
    ER r = E_OK;
    ID id = tskid;
    T_TCB *tcb;
    T_TCB **tbl;
    
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
        if (id != ID_ANY) {
            if (id == TSK_SELF) {
                _KERNEL_RET(_KERNEL_CHK_IS(s)? E_ID : E_OBJ);
            } else if (_kernel_tskid_max < (UINT)id) {
                _KERNEL_RET(E_ID);
            } else if (_KERNEL_CHK_TSK(s) && id == _kernel_cur->id) {
                _KERNEL_RET(E_OBJ);
            }
        }
        if (!pk_ctsk                                        ||
            !pk_ctsk->task                                  ||
            pk_ctsk->itskpri < 1                            ||
            (UINT)_kernel_tpri_max < (UINT)pk_ctsk->itskpri ||
            !pk_ctsk->stksz                                 ||
            pk_ctsk->stksz & TMSK_ALIGN                     ||
            (unsigned int)pk_ctsk->stk & TMSK_ALIGN          )
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * alloc auto id
     */
    if (id == ID_ANY) {
        id = _kernel_tskid_max;
        tbl = &_kernel_tcb[id - 1];
        for (; id && *tbl; id--, tbl--);
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        tbl = &_kernel_tcb[id - 1];
        if (*tbl)
            _KERNEL_END(E_OBJ);
    }
    /*
     * alloc TCB
     */
    if ((char *)pk_ctsk < _KERNEL_ETEXT()) {
        if (!(tcb = (T_TCB *)_kernel_sys_alloc(sizeof(T_TCB))))
            _KERNEL_END(E_SYS);
        tcb->ctsk = (T_CTSK *)pk_ctsk;
    } else {
        if (!(tcb = (T_TCB *)_kernel_sys_alloc(sizeof(T_TCB) + sizeof(T_CTSK))))
            _KERNEL_END(E_SYS);
        tcb->ctsk = (T_CTSK *)(tcb + 1);
        *tcb->ctsk = *pk_ctsk;
    }
    *tbl = tcb;
    /*
     * init TCB
     */
    tcb->tex = (VP)(tcb->tim.n = NULL);
    tcb->id = id;
    tcb->sts = TTS_DMT;
    tcb->act = 0;
    if (!pk_ctsk->stk) {
        /*
         * alloc task stack
         */
        VP new;
        if (!(new = _kernel_stk_alloc(pk_ctsk->stksz))) {
            _kernel_sys_free(tcb);
            _KERNEL_END(E_NOMEM);
        }
        tcb->stk = new;
    } else {
        tcb->stk = pk_ctsk->stk;
    }
#ifndef _KERNEL_NO_STKFIL
    /*
     * fill task stack by task id for user task stack level checking
     */
    memset(tcb->stk, id, pk_ctsk->stksz); /* task stack pattern : task id */
#endif
    /*
     * TA_ACT option: start task
     */
    if (pk_ctsk->tskatr & TA_ACT)
        _kernel_act(tcb, pk_ctsk->exinf);
    /*
     * acre_??? will return id
     */
    if (tskid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_TSK();
    return r;
}

/* eof */
