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
 * acp_por/pacp_por/tacp_por system call
 */
ER_UINT tacp_por(ID id, RDVPTN acpptn, RDVNO *p_rdvno, VP msg, TMO tmout)
{
    UINT psw;
    ER_UINT r = E_OK;
    T_POR *por;

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
        if (!id || _kernel_porid_max < (UINT)id)
            _KERNEL_RET(E_ID);
        if (!acpptn)
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
    if (!msg && por->cpor->maxcmsz)
        _KERNEL_END(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    {
        T_TCB *tcb, *cur;
        /*
         * look up call-rendezvous-waiting task and call pattern
         */
        {
            T_LNK *que = &por->cque[0]; 
            PRI pri = (por->cpor->poratr & TA_TPRI)? _kernel_tpri_max : 1;
            for (; pri; pri--, que++) {
                tcb = (T_TCB *)que->n;
                for (; tcb != (T_TCB *)que; tcb = tcb->n) {
                    RDVPTN calptn = _KERNEL_GET_TCB_PTN(tcb);
                    if (acpptn & calptn)
                        goto accept;
                }
            }
        }

        /*
         * accept-rendezvous-waiting
         */
        if (tmout == TMO_POL)
            _KERNEL_END_NOLOG(E_TMOUT);
        cur = (T_TCB *)_kernel_cur;
        cur->wai = TTW_ACP;
        cur->wid = id;
        _KERNEL_SET_REGPAR_DAT(msg);
        _KERNEL_SET_REGPAR_PTN(acpptn);
        _KERNEL_SET_REGPAR_FLG();
        r = _kernel_wai(cur, tmout, &por->aque, 0); /* dispatch */
        /*
         * get msg size and rendezvous number
         */
        if (0 < r)
            *p_rdvno = _KERNEL_GET_TCB_RDV(cur);
        goto end;

        /*
         * accept pattern matched
         */
    accept:
        /*
         * copy data
         */
        if ((r = _KERNEL_GET_TCB_SIZ(tcb)) != 0) {
            UB *dat = _KERNEL_GET_TCB_DAT(tcb);
            memcpy(msg, dat, r);
        }
        {
            /*
             * set rendezvous number
             */
            RDVNO rdv = ((_kernel_rdvseq++) << 8) | tcb->id;
            *p_rdvno = rdv;
            /*
             * update status of call-rendezvous-waiting task
             */
            tcb->wai = TTW_RDV;
            _KERNEL_SET_TCB_SIZ(tcb, por->cpor->maxrmsz);
            _KERNEL_SET_TCB_RDV(tcb, rdv);
            _kernel_deq((T_LNK *)tcb);
            _kernel_enq(&_kernel_dmy_que, (T_LNK *)tcb);
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TACP_POR();
    return r;
}

/* eof */
