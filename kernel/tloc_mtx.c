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
#ifdef _KERNEL_MTX
#include "kernel.h"

/*
 * loc_mtx/ploc_mtx/tloc_mtx system call
 */
ER tloc_mtx(ID id, TMO tmout)
{
    UINT psw;
    ER r = E_OK;
    T_MTX *mtx;
    T_TCB *cur = (T_TCB *)_kernel_cur;
    
#ifndef _KERNEL_NO_STATIC_ERR
    /*
     * check context
     */
    if (_KERNEL_CHK_WAI(_kernel_sts, tmout))
        _KERNEL_RET(E_CTX);
    /*
     * check par
     */
    if (!id || _kernel_mtxid_max < (UINT)id)
        _KERNEL_RET(E_ID);
#endif /* _KERNEL_NO_STATIC_ERR */

    /*
     * start critical section
     */
    psw = _KERNEL_DIS_PSW();
    /*
     * check lock counter
     */
    if (!(cur->mtx + 1))
        _KERNEL_END(E_QOVR); /* lock counter of the task is overflow */
    /*
     * lock mutex
     */
    if (!(mtx = _kernel_mtx[id - 1]))
        _KERNEL_END(E_NOEXS);
    if (!mtx->loc) {
        /*
         * TA_CEILING : raise current priority of current task
         */
        if (mtx->cmtx->mtxatr == TA_CEILING) {
            if (cur->bpri < mtx->cmtx->ceilpri)
                _KERNEL_END(E_ILUSE);
            if (mtx->cmtx->ceilpri < cur->pri) {
                cur->pri = mtx->cmtx->ceilpri;
                _kernel_deq((T_LNK *)cur);
                _kernel_enq(&_kernel_rdq[cur->pri - 1], (T_LNK *)cur);
            }
        }
        /*
         * lock
         */
        mtx->loc = cur;
        cur->mtx++;
    }
    /*
     * mutex waiting
     */
    else {
        if (mtx->loc == cur)    /* check lock nesting */
            _KERNEL_END(E_ILUSE);
        if (tmout == TMO_POL)
            _KERNEL_END_NOLOG(E_TMOUT);
        /*
         * TA_CEILING : check base priority of current task
         */
        if (mtx->cmtx->mtxatr == TA_CEILING) {
            if (cur->bpri < mtx->cmtx->ceilpri)
                _KERNEL_END(E_ILUSE);
        }
        /*
         * TA_INHERITE : raise current priority of lock task recursively
         */
        else if (mtx->cmtx->mtxatr == TA_INHERIT) {
            if (cur->pri < mtx->loc->pri)
                _kernel_chg_pri(mtx->loc, cur->pri);
        }
        /*
         * waiting
         */
        cur->wai = TTW_MTX;
        cur->wid = id;
        r = _kernel_wai(cur, tmout, &mtx->que[0], 
                        mtx->cmtx->mtxatr != TA_TFIFO); /* dispatch */
    }
    /*
     * end of critical section
     */
end:
    _KERNEL_SET_PSW(psw);
ret:
    _KERNEL_ASSERT_TLOC_MTX();
    return r;
}

#endif /* _KERNEL_MTX */
/* eof */
