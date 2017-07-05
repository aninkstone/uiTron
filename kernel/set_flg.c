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
 * set_flg/iset_flg system call
 */
ER set_flg(ID id, FLGPTN setptn)
{
    UINT psw;
    ER r = E_OK;
    T_FLG *flg;
    FLGPTN flgptn;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_flgid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * set flg
     */
    if (!(flg = _kernel_flg[id - 1]))
        _KERNEL_END(E_NOEXS);
    flgptn = flg->flgptn |= setptn;
    /*
     * waiting pattern match
     */
    {
        PRI pri = (!(~flg->cflg->flgatr & (TA_WMUL | TA_TPRI)))? _kernel_tpri_max : 1;
        T_LNK *que = &flg->que[0];
        B dsp = 0;
        for (; pri; pri--, que++) {
            T_TCB *tcb = (T_TCB *)que->n;
            while (tcb != (T_TCB *)que) {
                T_TCB *tcb_n = tcb->n;
                FLGPTN wptn = _KERNEL_GET_TCB_PTN(tcb);
                MODE   wmod = _KERNEL_GET_TCB_MOD(tcb);
                FLGPTN mptn  = flgptn & wptn;
                /*
                 * match : wake up
                 */
                if ((wmod & TWF_ORW)? mptn : (mptn == wptn)) {
                    _KERNEL_SET_TCB_PTN(tcb, flgptn);
                    _KERNEL_SET_TCB_RET(tcb, E_OK);
                    dsp |= _kernel_deq_rdy(tcb);
                    /*
                     * clear option
                     */
                    if (wmod & TWF_CLR || flg->cflg->flgatr & TA_CLR) {
                        flg->flgptn = 0;
                        goto disp;
                    }
                }
                tcb = tcb_n;
            }
        }
disp:
        if (dsp)
            _kernel_highest();  /* dispatch */
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_SET_FLG();
    return r;
}

/* eof */
