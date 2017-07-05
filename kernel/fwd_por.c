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
 * fwd_por system call
 */
ER fwd_por(ID id, RDVPTN calptn, RDVNO rdvno, VP msg, UINT cmsgsz)
{
    UINT psw;
    ER r = E_OK;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_porid_max < (UINT)id)
        _KERNEL_RET(E_ID);
    if (!calptn || (!msg && cmsgsz))
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * forward rendezvous
     */
    {
        T_POR *src, *dst;
        T_TCB *cal, *acp;
        RDVNO rdv;

        /*
         * get source/destination rendezvous port and call task
         */
        if (!(dst = _kernel_por[id - 1]))
            _KERNEL_END(E_NOEXS);
        {
            ID tid = rdvno & 0xff;
            if (!tid                              ||
                _kernel_tskid_max < (UINT)tid     ||
                !(cal = _kernel_tcb[tid - 1])     ||
                !(cal->sts & TTS_WAI)             ||
                cal->wai != TTW_RDV               ||
                _KERNEL_GET_TCB_RDV(cal) != rdvno ||
                !(src = _kernel_por[cal->wid - 1]) )
                _KERNEL_END(E_OBJ);
        }
        /*
         * check msg size
         */
        {
            UINT smaxrmsz = src->cpor->maxrmsz;
            if (smaxrmsz < dst->cpor->maxrmsz)
                _KERNEL_END(E_ILUSE);
            if (dst->cpor->maxcmsz < cmsgsz || smaxrmsz < cmsgsz  )
                _KERNEL_END(E_PAR);
        }
        /*
         * look up accept-rendezvous-waiting task and accept pattern
         */
        {
            T_LNK *que = &dst->aque; 
            acp = (T_TCB *)que->n;
            for (; acp != (T_TCB *)que; acp = acp->n) {
                RDVPTN acpptn = _KERNEL_GET_TCB_PTN(acp);
                if (acpptn & calptn)
                    goto accept;
            }
        }

        /*
         * copy data
         */
        if (cmsgsz) {
            UB *dat = _KERNEL_GET_TCB_DAT(cal);
            memcpy(dat, msg, cmsgsz);
        }
        /*
         * call task : reply-rendezvous-waiting -> call-rendezvous-waiting
         */
        cal->wai = TTW_CAL;
        _KERNEL_SET_TCB_SIZ(cal, cmsgsz);
        _KERNEL_SET_TCB_PTN(cal, calptn);
        _kernel_deq((T_LNK *)cal);
        {
            T_LNK *que = &dst->cque[0];
            if (dst->cpor->poratr & TA_TPRI)
                que += (cal->pri - 1);
            _kernel_enq(que, (T_LNK *)cal);
        }
        goto end;

        /*
         * accept pattern matched
         */
    accept:
        /*
         * copy data
         */
        if (cmsgsz) {
            UB *dat = _KERNEL_GET_TCB_DAT(acp);
            memcpy(dat, msg, cmsgsz);
        }
        /*
         * set rendezvous number
         */
        rdv = ((_kernel_rdvseq++) << 8) | cal->id;
        _KERNEL_SET_TCB_RDV(cal, rdv);
        _KERNEL_SET_TCB_RDV(acp, rdv);
        /*
         * wake up accept-rendezvous-waiting task
         */
        _KERNEL_SET_TCB_RET(acp, cmsgsz);
        _kernel_deq_rdy_dsp(acp); /* dispatch */
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_FWD_POR();
    return r;
}

/* eof */
