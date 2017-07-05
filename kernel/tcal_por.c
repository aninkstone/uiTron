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
 * cal_por/tcal_por system call
 */
ER_UINT tcal_por(ID id, RDVPTN calptn, VP msg, UINT cmsgsz, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_POR *por;

#ifndef _KERNEL_NO_STATIC_ERR
    {
        UB s = _kernel_sts;
        /*
         * check context
         */
        if (tmout == TMO_POL) {
            _KERNEL_RET(E_PAR);
        } else {
            if (_KERNEL_CHK_WAI(s, tmout))
                _KERNEL_RET(E_CTX);
        }
        /*
         * check par
         */
        if (!id || _kernel_porid_max < (UINT)id)
            _KERNEL_RET(E_ID);
        if (!calptn || (!msg && cmsgsz))
            _KERNEL_RET(E_PAR);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * call rendezvous
     */
    if (!(por = _kernel_por[id - 1]))
        _KERNEL_END(E_NOEXS);

#ifndef _KERNEL_NO_STATIC_ERR
    if (por->cpor->maxrmsz < cmsgsz)
        _KERNEL_END(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    {
        T_TCB *tcb, *cur;
        RDVNO rdv;
        /*
         * look up accept-rendezvous waiting task and accept pattern
         */
        {
            T_LNK *que = &por->aque; 
            tcb = (T_TCB *)que->n;
            for (; tcb != (T_TCB *)que; tcb = tcb->n) {
                RDVPTN acpptn = _KERNEL_GET_TCB_PTN(tcb);
                if (acpptn & calptn)
                    goto accept;
            }
        }

        /*
         * call-rendezvous waiting
         */
        cur = (T_TCB *)_kernel_cur;
        cur->wai = TTW_CAL;
        cur->wid = id;
        _KERNEL_SET_REGPAR_DAT(msg);
        _KERNEL_SET_REGPAR_SIZ(cmsgsz);
        _KERNEL_SET_REGPAR_PTN(calptn);
        _KERNEL_SET_REGPAR_FLG();
        r = _kernel_wai(cur, tmout, &por->cque[0],
                        por->cpor->poratr & TA_TPRI); /* dispatch */
        goto end;

        /*
         * accept pattern matched
         */
    accept:
        /*
         * copy data
         */
        if (cmsgsz) {
            UB *dat = _KERNEL_GET_TCB_DAT(tcb);
            memcpy(dat, msg, cmsgsz);
        }
        /*
         * set rendezvous number
         */
        cur = (T_TCB *)_kernel_cur;
        rdv = ((_kernel_rdvseq++) << 8) | cur->id;
        _KERNEL_SET_TCB_RDV(tcb, rdv);
        /*
         * wake up accept-rendezvous waiting task
         */
        _KERNEL_SET_TCB_RET(tcb, cmsgsz);
        _kernel_deq_rdy(tcb);
        /*
         * reply-rendezvous waiting
         */
        cur->wai = TTW_RDV;
        cur->wid = id;
        _KERNEL_SET_REGPAR_DAT(msg);
        _KERNEL_SET_REGPAR_SIZ(por->cpor->maxrmsz);
        _KERNEL_SET_REGPAR_RDV(rdv);
        _KERNEL_SET_REGPAR_FLG();
        r = _kernel_wai(cur, tmout, &_kernel_dmy_que, 0); /* dispatch */
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TCAL_POR();
    return r;
}

/* eof */
