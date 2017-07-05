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
#ifdef _KERNEL_OVR
#include "kernel.h"

static FP ovrclr _KERNEL_DATA = NULL;
static ID isrid _KERNEL_DATA = 0;

/*
 * ISR for OVR
 */
static void chkovr(VP_INT exinf, INHNO inhno)
{
    /*
     * user task is running : update counter
     */
    if (_kernel_cur && !(_kernel_sts & T_KER_SYS)) {
        OVRTIM *ptr = (OVRTIM *)&_kernel_ovrcnt[_kernel_cur->id - 1];
        OVRTIM cnt = *ptr;
        if (cnt && !(cnt + 1)) { /* not 0 and not 0xffffffff */
            if (!(--cnt)) {      /* overrun */
                --cnt;           /* cnt = 0xffffffff : overrun mark */
                _kernel_ovrreq = 1; /* request overrun handler */
            }
            *ptr = cnt;
        }
    }
    /*
     * clear interrupt
     */
    if (ovrclr)
        (*ovrclr)();
}

/*
 * def_ovr system call
 */
ER def_ovr(T_DOVR *pk_dovr)
{
    UINT psw;
    ER r = E_OK;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_IL(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (pk_dovr && (!pk_dovr->ovrhdr                 ||
                    pk_dovr->intno < 0               ||
                    _KERNEL_IRQ_CNT <= pk_dovr->intno ))
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    if (pk_dovr) {
        /*
         * 1st define
         */
        if (!_kernel_ovrcnt) {
            int i;
            OVRTIM *cnt;
            T_CISR cisr;
            ER_ID id;
            /*
             * allocate time counter table
             */
            if (!(cnt = (OVRTIM *)_kernel_sys_alloc(
                      sizeof(OVRTIM) * _kernel_tskid_max)))
                _KERNEL_END(E_SYS);
            _kernel_ovrcnt = cnt;
            /*
             * stop all counter
             */
            for (i = _kernel_tskid_max; i--; )
                *cnt++ = 0;
            /*
             * create ISR
             */
            cisr.isratr = TA_HLNG;
            cisr.exinf  = 0;
            cisr.intno  = pk_dovr->intno;
            cisr.isr    = (FP)chkovr;
            cisr.imask  = pk_dovr->imask;
            if ((id = acre_isr(&cisr)) < 0)
                _KERNEL_END(id);
            isrid = id;
        }
        /*
         * redefine
         */
        else {
            int i;
            OVRTIM *cnt = (OVRTIM *)_kernel_ovrcnt;
            for (i = _kernel_tskid_max; i--;)
                *cnt++ = 0;
        }
        _kernel_ovrhdr = pk_dovr->ovrhdr;
        ovrclr = pk_dovr->ovrclr;
    }
    /*
     * delete
     */
    else if (_kernel_ovrcnt) {
        _kernel_sys_free((VP)_kernel_ovrcnt);
        _kernel_ovrcnt = NULL;
        _kernel_ovrreq = 0;
        _kernel_ovrhdr = NULL;
        ovrclr = NULL;
        r = del_isr(isrid);
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_DEF_OVR();
    return r;
}

#endif /* _KERNEL_OVR */
/* eof */
