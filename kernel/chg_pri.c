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

#ifdef _KERNEL_MTX
/*
 * change priority of the task which is mutex waiting or
 * is inherited by mutex
 *   return value :
 *     1 : require dispatch
 *     0 : not require
 */
B _kernel_chg_mtx(T_TCB *tcb, PRI pri)
{
    B dsp = 0;
    T_MTX *mtx ;

    mtx = _kernel_mtx[tcb->wid - 1];
    if (mtx->cmtx->mtxatr == TA_TFIFO)
        goto ret;
    /*
     * change waiting queue
     */
    _kernel_deq((T_LNK *)tcb);
    _kernel_enq(&mtx->que[pri - 1], (T_LNK *)tcb);
    /*
     * raise inherited task priority
     */
    if (mtx->cmtx->mtxatr == TA_INHERIT && pri < mtx->loc->pri) {
        /*
         * mark inherited task to inherit table
         */
        memset(_kernel_inherit, 0, _kernel_tskid_max);
        _kernel_inherit[tcb->id - 1] = 1; /* first mark */
        for (;;) {
            tcb = mtx->loc;
            if (_kernel_inherit[tcb->id - 1])
                break;          /* looped inheritance : stop */
            tcb->pri = pri;
            _kernel_inherit[tcb->id - 1] = 1;
            /*
             * mutex waiting
             */
            if (tcb->sts & TTS_WAI && tcb->wai == TTW_MTX) {
                mtx = _kernel_mtx[tcb->wid - 1];
                if (mtx->cmtx->mtxatr != TA_TFIFO) {
                    _kernel_deq((T_LNK *)tcb);
                    _kernel_enq(&mtx->que[pri - 1], (T_LNK *)tcb);
                    if (mtx->cmtx->mtxatr == TA_INHERIT &&
                        pri < mtx->loc->pri)
                        continue;
                }
            }
            /*
             * not mutex waiting, but inherited
             */
            else {
                dsp |= _kernel_chg_pri(tcb, pri); /* recursive call */
            }
            break;
        }
    }
ret:
    return dsp;
}
#endif /* _KERNEL_MTX */

/*
 * change current priority of the task
 *   return value :
 *     1 : require dispatch
 *     0 : not require
 */
B _kernel_chg_pri(T_TCB *tcb, PRI pri)
{
    B dsp = 0;

#ifdef _KERNEL_MTX
    /*
     * cancel priority down if the task locked CEILING/INHERIT mutex
     */
    if (tcb->pri <= pri) {
        T_MTX **mtxp = &_kernel_mtx[0];
        UINT i = _kernel_mtxid_max;
        for (; i--; mtxp++) {
            if (*mtxp &&
                (*mtxp)->loc == tcb &&
                (*mtxp)->cmtx->mtxatr & TA_INHERIT) /* CEILING or INHERIT */
                goto ret;
        }
    }
#endif /* _KERNEL_MTX */

    /*
     * change priority by the task status
     */
    {
        PRI curpri = (_kernel_cur)? _kernel_cur->pri : 255;
        tcb->pri = pri;

        switch (tcb->sts) {
        case TTS_RDY :
            /*
             * READY : update ready queue
             */
            _kernel_deq((T_LNK *)tcb);
            _kernel_enq(&_kernel_rdq[pri - 1], (T_LNK *)tcb);
            if (tcb == _kernel_cur) {
                if (curpri <= pri)
                    dsp = 1; /* current task priority is down */
            } else {
                if (pri < curpri)
                    dsp = 1; /* another task priority is higher than current */
            }
            break;

        case TTS_WAI :
        case TTS_WAS :
            /*
             * WAITING or WAITING-SUSPENDED
             */
            switch (tcb->wai) {
            case TTW_SEM :
                {
                    T_SEM *sem = _kernel_sem[tcb->wid - 1];
                    if (sem->csem->sematr & TA_TPRI) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&sem->que[pri - 1], (T_LNK *)tcb);
                    }
                }
                break;
            case TTW_FLG :
                {
                    T_FLG *flg = _kernel_flg[tcb->wid - 1];
                    if (!(~flg->cflg->flgatr & (TA_WMUL | TA_TPRI))) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&flg->que[pri - 1], (T_LNK *)tcb);
                    }
                }
                break;
            case TTW_SDTQ :
                {
                    T_DTQ *dtq = _kernel_dtq[tcb->wid - 1];
                    if (dtq->cdtq->dtqatr & TA_TPRI) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&dtq->sque[pri - 1], (T_LNK *)tcb);
                    }
                }
                break;
            case TTW_MBX :
                {
                    T_MBX *mbx = _kernel_mbx[tcb->wid - 1];
                    if (mbx->cmbx->mbxatr & TA_TPRI) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&mbx->que[pri - 1], (T_LNK *)tcb);
                    }
                }
                break;

#ifdef _KERNEL_MTX
            case TTW_MTX :
                dsp = (*_kernel_chg_mtx_fp)(tcb, pri);
                break;
#endif /* _KERNEL_MTX */

            case TTW_SMBF :
                {
                    T_MBF *mbf = _kernel_mbf[tcb->wid - 1];
                    if (mbf->cmbf->mbfatr & TA_TPRI) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&mbf->sque[pri - 1], (T_LNK *)tcb);
                    }
                    dsp = (*_kernel_chk_wmbf_fp)(mbf);
                }
                break;
            case TTW_RMBF :
                {
                    T_MBF *mbf = _kernel_mbf[tcb->wid - 1];
                    if (mbf->cmbf->mbfatr & TA_TPRIR) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&mbf->sque[_kernel_tpri_max + pri - 1], (T_LNK *)tcb);
                    }
                }
                break;
            case TTW_CAL :
                {
                    T_POR *por = _kernel_por[tcb->wid - 1];
                    if (por->cpor->poratr & TA_TPRI) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&por->cque[pri - 1], (T_LNK *)tcb);
                    }
                }
                break;
            case TTW_MPF :
                {
                    T_MPF *mpf = _kernel_mpf[tcb->wid - 1];
                    if (mpf->cmpf->mpfatr & TA_TPRI) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&mpf->que[pri - 1], (T_LNK *)tcb);
                    }
                }
                break;
            case TTW_MPL :
                {
                    T_MPL *mpl = _kernel_mpl[tcb->wid - 1];
                    if (mpl->cmpl->mplatr & TA_TPRI) {
                        _kernel_deq((T_LNK *)tcb);
                        _kernel_enq(&mpl->que[pri - 1], (T_LNK *)tcb);
                    }
                    dsp = (*_kernel_chk_wmpl_fp)(mpl);
                }
                break;
            default :
                /* TTW_SLP/TTW_DLY/TTW_RDTQ/TTW_ACP/TTW_RDV */
                break;
            }
            break;
        default :
            /* TTS_SUS/TTS_DMT */
            break;
        }
    }
ret:
    return dsp;
}

#ifdef _KERNEL_MTX
/*
 * check ceiling mutex priority
 *   return value :
 *     E_OK    : ok
 *     E_ILUSE : the base priority over ceiling priority
 */
static __inline__ ER __attribute__ ((always_inline))
chk_pri(T_TCB *tcb, PRI bpri)
{
    /*
     * check waiting mutex
     */
    if (tcb->sts & TTS_WAI && tcb->wai == TTW_MTX) {
        T_MTX *mtx = _kernel_mtx[tcb->wid - 1];
        if (mtx->cmtx->mtxatr == TA_CEILING && bpri < mtx->cmtx->ceilpri)
            return E_ILUSE;
    }
    /*
     * check locked mutex
     */
    {
        T_MTX **mtxp = &_kernel_mtx[0];
        UINT i = _kernel_mtxid_max;
        for (; i--; mtxp++) {
            if (*mtxp &&
                (*mtxp)->loc == tcb &&
                (*mtxp)->cmtx->mtxatr == TA_CEILING &&
                bpri < (*mtxp)->cmtx->ceilpri)
                return E_ILUSE;
        }
    }
    return E_OK;
}
#endif /* _KERNEL_MTX */

/*
 * chg_pri system call
 */
ER chg_pri(ID tskid, PRI tskpri)
{
    UINT psw;
    ER r = E_OK;
    T_TCB *tcb;

#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_LOC(_kernel_sts))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (tskid == TSK_SELF) {
        if (_KERNEL_CHK_IS(_kernel_sts))
            _KERNEL_RET(E_ID);
    } else if (_kernel_tskid_max < (UINT)tskid) {
        _KERNEL_RET(E_ID);
    }
    if ((UINT)_kernel_tpri_max < (UINT)tskpri)
        _KERNEL_RET(E_PAR);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();

#ifdef _KERNEL_MTX
    /*
     * check ceiling mutex
     */
    if (!(tcb = _kernel_tcb[tskid - 1]))
        _KERNEL_END(E_NOEXS);
    if (tskpri < tcb->pri) {
        if ((r = chk_pri(tcb, tskpri)) != E_OK)
            _KERNEL_END(r);     /* E_ILUSE */
    }
    /*
     * change base priority of the task
     */
    tcb->bpri = tskpri;
#endif /* _KERNEL_MTX */

    /*
     * change current priority of the task
     */
    if (_kernel_chg_pri(tcb, tskpri))
        _kernel_highest();      /* dispatch */
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_CHG_PRI();
    return r;
}

/* end */
