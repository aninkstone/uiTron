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
 * interrupt handler for ISR
 */
static void isrhdr(INHNO inhno)
{
    T_LNK *que = &_kernel_isrq[(UH)inhno & _kernel_isrq_msk];
    T_ISR *isr;

    for (isr = (T_ISR *)que->n; isr != (T_ISR *)que; isr = (T_ISR *)isr->n) {
        if (isr->intno == inhno) {
#ifdef _KERNEL_INT_IMASK
            _KERNEL_SET_PSW(isr->imask);
#endif
            (*isr->isr)(isr->exinf, inhno);
#ifdef _KERNEL_INT_IMASK
            _KERNEL_DIS();
#endif
        }
    }
}

/*
 * cre_isr/acre_isr system call
 */
ER cre_isr(ID isrid, const T_CISR *pk_cisr)
{
    UINT psw;
    ER r = E_OK;
    ID id = isrid;
    T_ISR *isr;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (id != ID_ANY) {
        if (!id || _kernel_isrid_max < (UINT)id)
            _KERNEL_RET(E_ID);
    }
    if (!pk_cisr || (id != ID_ANY && _KERNEL_IRQ_CNT <= pk_cisr->intno)
        || !pk_cisr->isr)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * check id
     */
    if (id == ID_ANY) {
        for (id = _kernel_isrid_max; _kernel_isr[id - 1] && id > 0; id--)
            ;
        if (!id)
            _KERNEL_END(E_NOID);
    } else {
        if (_kernel_isr[id - 1])
            _KERNEL_END(E_OBJ);
    }
    /*
     * alloc ISR struct
     */
    if (!(isr = (T_ISR *)_kernel_sys_alloc(sizeof(T_ISR))))
        _KERNEL_END(E_SYS);
    _kernel_isr[id - 1] = isr;
    /*
     * init ISR struct
     */
    isr->isr   = (FP_ISR)pk_cisr->isr;
    isr->exinf = pk_cisr->exinf;
    isr->intno = pk_cisr->intno;
#ifdef _KERNEL_INT_IMASK
    isr->imask = pk_cisr->imask;
#endif
    /*
     * define interrupt handler for ISRs
     */
    {
        T_DINH dinh;
        dinh.inhatr = TA_HLNG;
        dinh.inthdr = (FP)isrhdr;
#ifdef _KERNEL_INT_IMASK
        dinh.imask  = _kernel_msk;
#endif
        if ((r = def_inh(pk_cisr->intno, &dinh)) != E_OK) {
            _kernel_sys_free(isr);
            goto end;
        }
    }
    /*
     * link ISR struct to ISR QUEUE
     */
    _kernel_enq(&_kernel_isrq[(UH)pk_cisr->intno & _kernel_isrq_msk],
                (T_LNK *)isr);
    /*
     * acre_??? will return id
     */
    if (isrid == ID_ANY)
        r = id;
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CRE_ISR();
    return r;
}

/* eof */
