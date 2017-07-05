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
 * get msg data from ring buffer
 *   return value :
 *       not 0 : msg size
 *       0     : no msg
 */
static UINT _kernel_get_mbf(T_MBF *mbf, UB *msg)
{
    if (mbf->smsgcnt) {
        SIZE mbfsz = mbf->cmbf->mbfsz;
        UB *end = mbf->buf + mbfsz;

        if (mbf->cmbf->maxmsz == 1) {
            /*
             * get byte msg
             */
            *msg = *mbf->out;
            if (++mbf->out == end)
                mbf->out = mbf->buf;
            mbf->sz--;
            mbf->smsgcnt--;
            return 1;           /* success */
        } else {
            UINT msgsz;
            UINT tmpsz;

            /*
             * get size header
             */
            msgsz = ((UINT)*mbf->out) << 8;
            if (++mbf->out == end)
                mbf->out = mbf->buf;
            msgsz += (UINT)*mbf->out;
            if (++mbf->out == end)
                mbf->out = mbf->buf;
            /*
             * get msg data
             */
            tmpsz = end - mbf->out;
            if (msgsz < tmpsz) {
                memcpy(msg, mbf->out, msgsz);
                mbf->out += msgsz;
            } else if (msgsz == tmpsz) {
                memcpy(msg, mbf->out, msgsz);
                mbf->out = mbf->buf;
            } else {
                memcpy(msg, mbf->out, tmpsz);
                memcpy(msg + tmpsz, mbf->buf, msgsz - tmpsz);
                mbf->out = mbf->buf + (msgsz - tmpsz);
            }
            mbf->sz -= (msgsz + 2);
            mbf->smsgcnt--;
            return msgsz;       /* success */
        }
    }
    return 0;                   /* no msg */
}

/*
 * wakeup send MBF waiting task
 *   return value :
 *     1 : require dispatch
 *     0 : not require
 */
B _kernel_chk_wmbf(T_MBF *mbf)
{
    B dsp = 0;
    PRI spri;
    T_LNK *que;
    
    /*
     * put send-waiting-task's data to buffer
     */
    spri = (mbf->cmbf->mbfatr & TA_TPRI)? _kernel_tpri_max : 1;
    que = &mbf->sque[0];
    for (; spri--; que++) {
        T_TCB *tcb = (T_TCB *)que->n;
        while (tcb != (T_TCB *)que) {
            T_TCB *tcb_n = tcb->n;
            UB  *dat = _KERNEL_GET_TCB_DAT(tcb);
            UINT siz = _KERNEL_GET_TCB_SIZ(tcb);
            if (!_kernel_put_mbf(mbf, dat, siz))
                goto end;   /* no more space */
            /*
             * wake up send waiting task
             */
            _KERNEL_SET_TCB_RET(tcb, E_OK);
            dsp |= _kernel_deq_rdy(tcb);
            tcb = tcb_n;
        }
    }
end:
    return dsp;
}

/*
 * rcv_mbf/prcv_mbf/trcv_mbf system call
 */
ER_UINT trcv_mbf(ID id, VP msg, TMO tmout)
{
    UINT psw;
    ER_UINT r = 0;
    T_MBF *mbf;

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
        if (!id || _kernel_mbfid_max < (UINT)id)
            _KERNEL_RET(E_ID);
    }
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * get bufferd data
     */
    if (!(mbf = _kernel_mbf[id - 1]))
        _KERNEL_END(E_NOEXS);
    if ((r = _kernel_get_mbf(mbf, (UB *)msg)) != 0) {
        if (_kernel_chk_wmbf(mbf))
            _kernel_highest();  /* dispatch */
    }
    /*
     * receive directly
     */
    else {
        PRI spri = (mbf->cmbf->mbfatr & TA_TPRI)? _kernel_tpri_max : 1;
        T_TCB *tcb;
        UB *dat;
        /*
         * receive from send-waiting-task : wake up
         */
        if ((tcb = (T_TCB *)_kernel_first(&mbf->sque[0], spri)) != NULL) {
            dat = _KERNEL_GET_TCB_DAT(tcb);
            if ((r = _KERNEL_GET_TCB_SIZ(tcb)) == 1) {
                *((UB *)msg) = *dat;
            } else {
                memcpy(msg, dat, r);
            }
            _KERNEL_SET_TCB_RET(tcb, E_OK);
            _kernel_deq_rdy_dsp(tcb); /* dispatch */
        }
        /*
         * receive waiting
         */
        else {
            if (tmout == TMO_POL) {
                r = E_TMOUT;
            } else {
                tcb = (T_TCB *)_kernel_cur;
                tcb->wai = TTW_RMBF;
                tcb->wid = id;
                r = _kernel_wai(tcb, tmout, &mbf->sque[spri],
                                mbf->cmbf->mbfatr & TA_TPRIR); /* dispatch */
                /*                          &mbf->rque[0] */
                if (0 < r) {
                    /*
                     * get data
                     */
                    dat = _KERNEL_GET_TCB_DAT(tcb);
                    if (r == 1) {
                        *((UB *)msg) = *dat;
                    } else {
                        memcpy(msg, dat, r);
                    }
                }
            }
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TRCV_MBF();
    return r;
}

/* eof */
