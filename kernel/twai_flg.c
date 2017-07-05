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
 * wai_flg/pol_flg/twai_flg system call
 */
ER twai_flg(ID id, FLGPTN waiptn, MODE wfmode, FLGPTN*p_flgptn, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_FLG *flg;
    
#ifndef _KERNEL_NO_STATIC_ERR
    {
        UB s = _kernel_sts;
        /*
         * check context
         */
        if (tmout == TMO_POL) {
            if (_KERNEL_CHK_LOC(s))
                _KERNEL_RET(E_CTX);
        } else {
            if (_KERNEL_CHK_WAI(s, tmout))
                _KERNEL_RET(E_CTX);
        }
        /*
         * check par
         */
        if (!id || _kernel_flgid_max < (UINT)id)
            _KERNEL_RET(E_ID);
        if (!waiptn || !p_flgptn)
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * check flag pattern
     */
    if (!(flg = _kernel_flg[id - 1]))
        _KERNEL_END(E_NOEXS);
    {
        FLGPTN mptn = waiptn & flg->flgptn;
        /*
         * matched : no wait case
         */
        if ((wfmode & TWF_ORW)? mptn : (mptn == waiptn)) {
            *p_flgptn = flg->flgptn;
            if (wfmode & TWF_CLR || flg->cflg->flgatr & TA_CLR)
                flg->flgptn = 0;
        }
        /*
         * unmatched : eventflag waiting
         */
        else {
            T_LNK *que = &flg->que[0];
            if (!(flg->cflg->flgatr & TA_WMUL) && que->n != que) /* TA_WSGL */
                _KERNEL_END(E_ILUSE);
            if (tmout == TMO_POL) {
                r = E_TMOUT;
            } else {
                T_TCB *tcb = (T_TCB *)_kernel_cur;
                tcb->wai = TTW_FLG;
                tcb->wid = id;
                _KERNEL_SET_REGPAR_PTN(waiptn);
                _KERNEL_SET_REGPAR_MOD(wfmode);
                _KERNEL_SET_REGPAR_FLG();
                r = _kernel_wai(tcb, tmout, que,
                                !(~flg->cflg->flgatr & (TA_WMUL | TA_TPRI))); /* dispatch */
                /*                                      WMUL and TPRI */
                if (r == E_OK)
                    *p_flgptn = _KERNEL_GET_TCB_PTN(tcb);
            }
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TWAI_FLG();
    return r;
}

/* eof */
