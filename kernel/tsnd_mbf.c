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
 * put msg data to ring buffer
 *   return value :
 *       not 0 : msg size
 *       0     : no space
 */
UINT _kernel_put_mbf(T_MBF *mbf, UB *msg, UINT msgsz)
{
    SIZE spc = mbf->cmbf->mbfsz - mbf->sz;
    UB *end = mbf->buf + mbf->cmbf->mbfsz;
    
    if (mbf->cmbf->maxmsz == 1) {
        if (spc) {
            /*
             * store byte msg
             */
            *mbf->in = *msg;
            if (++mbf->in == end)
                mbf->in = mbf->buf;
            mbf->sz++;
            mbf->smsgcnt++;
            return 1;           /* success */
        }
    } else {
        if (msgsz + 2 <= spc) {
            UINT tmpsz;
            /*
             * store size header
             */
            *mbf->in = (UB)(msgsz >> 8);
            if (++mbf->in == end)
                mbf->in = mbf->buf;
            *mbf->in = (UB)msgsz;
            if (++mbf->in == end)
                mbf->in = mbf->buf;
            /*
             * store msg data
             */
            tmpsz = end - mbf->in;
            if (msgsz < tmpsz) {
                memcpy(mbf->in, msg, msgsz);
                mbf->in += msgsz;
            } else if (msgsz == tmpsz) {
                memcpy(mbf->in, msg, msgsz);
                mbf->in = mbf->buf;
            } else {
                memcpy(mbf->in, msg, tmpsz);
                memcpy(mbf->buf, ((UB *)msg) + tmpsz, msgsz - tmpsz);
                mbf->in = mbf->buf + (msgsz - tmpsz);
            }
            mbf->sz += (msgsz + 2);
            mbf->smsgcnt++;
            return msgsz;       /* success */
        }
    }
    return 0;                   /* no space */
}

/*
 * snd_mbf/psnd_mbf/tsnd_mbf system call
 */
ER tsnd_mbf(ID id, VP msg, UINT msgsz, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_MBF *mbf;
    UB s = _kernel_sts;

#ifndef _KERNEL_NO_STATIC_ERR
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
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * send data
     */
    if (!(mbf = _kernel_mbf[id - 1]))
        _KERNEL_END(E_NOEXS);

#ifndef _KERNEL_NO_STATIC_ERR
    if (!msgsz || mbf->cmbf->maxmsz < msgsz)
        _KERNEL_END(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    {
        PRI spri = (mbf->cmbf->mbfatr & TA_TPRI )? _kernel_tpri_max : 1;
        PRI rpri = (mbf->cmbf->mbfatr & TA_TPRIR)? _kernel_tpri_max : 1;
        /*
         * no bufferd data and receive-waiting-task exist : wake up
         */
        if (!mbf->sz) {
            T_TCB *tcb = (T_TCB *)_kernel_first(&mbf->sque[spri], rpri);
            /*                                  &mbf->rque[0] */
            if (tcb) {
                _KERNEL_SET_TCB_DAT(tcb, msg);
                _KERNEL_SET_TCB_RET(tcb, msgsz);
                _kernel_deq_rdy_dsp(tcb); /* dispatch */
                goto end;
            }
        }
        /*
         * store data to buffer
         */
        if (_KERNEL_CHK_IS(s)) {
            if (!_kernel_put_mbf(mbf, (UB *)msg, msgsz))
                _KERNEL_END_NOLOG(E_TMOUT);
        } else {                /* task context */
            T_TCB *tcb = (T_TCB *)_kernel_cur;
            PRI pri = (mbf->cmbf->mbfatr & TA_TPRI)? tcb->pri : 1;
            if (!_kernel_first(&mbf->sque[0], pri)) {
                if (_kernel_put_mbf(mbf, (UB *)msg, msgsz))
                    goto end;
            }
            /*
             * not enough buffer space : send waiting
             */
            if (tmout == TMO_POL) {
                r = E_TMOUT;
            } else {
                tcb->wai = TTW_SMBF;
                tcb->wid = id;
                _KERNEL_SET_REGPAR_DAT(msg);
                _KERNEL_SET_REGPAR_SIZ(msgsz);
                _KERNEL_SET_REGPAR_FLG();
                r = _kernel_wai(tcb, tmout, &mbf->sque[0],
                                mbf->cmbf->mbfatr & TA_TPRI); /* dispatch */
            }
        }
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TSND_MBF();
    return r;
}

/* eof */
